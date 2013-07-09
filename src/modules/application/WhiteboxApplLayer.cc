/***************************************************************************
 * file:        WhiteboxApplLayer.h
 **************************************************************************/

#include "WhiteboxApplLayer.h"

#include <sstream>

#include "BaseNetwLayer.h"
#include "AddressingInterface.h"
#include "NetwControlInfo.h"
#include "FindModule.h"
#include "SimpleAddress.h"
#include "MacToPhyInterface.h"
#include "BaseWorldUtility.h"
#include "SensorPkt_m.h"
#include "ApplPkt_m.h"

Define_Module(WhiteboxApplLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void WhiteboxApplLayer::initialize(int stage) {
    BaseLayer::initialize(stage);
    if (stage == 0) {
        BaseLayer::catPacketSignal.initialize();

        USBLayerIn  = findGate("USBLayerIn");
        USBLayerOut = findGate("USBLayerOut");
        USBControlIn  = findGate("USBControlIn");
        USBControlOut = findGate("USBControlOut");

        debugEV<< "in initialize() stage 0...";
        debug = par("debug");
        stats = par("stats");
        trace = par("trace");
        nbPackets = par("nbPackets");
        trafficParam = par("trafficParam");
        initializationTime = par("initializationTime");
        broadcastPackets = par("broadcastPackets");
        headerLength = par("headerLength");
        // application configuration
        const char *traffic = par("trafficType");
        destAddr = LAddress::L3Type(par("destAddr").longValue());
        nbPacketsSent = 0;
        nbPacketsReceived = 0;
        firstPacketGeneration = -1;
        lastPacketReception = -2;

        controlID = 0;

        sleepAfterFinish = par("sleepAfterFinish").boolValue();

        initializeDistribution(traffic);

        delayTimer = new cMessage("appDelay", SEND_DATA_TIMER);
        turnOffTimer = new cMessage("appDelay", TURN_RADIO_OFF_TIMER);

        // get pointer to the world module
        world = FindModule<BaseWorldUtility*>::findGlobalModule();

    } else if (stage == 1) {
        debugEV << "in initialize() stage 1...";
        // Application address configuration: equals to host address

        cModule *const pHost = findHost();
        const cModule* netw  = FindModule<BaseNetwLayer*>::findSubModule(pHost);
        if(!netw) {
            netw = pHost->getSubmodule("netwl");
            if(!netw) {
                opp_error("Could not find network layer module. This means "
                          "either no network layer module is present or the "
                          "used network layer module does not subclass from "
                          "BaseNetworkLayer.");
            }
        }
        const AddressingInterface *const addrScheme = FindModule<AddressingInterface*>::findSubModule(pHost);
        if(addrScheme) {
            myAppAddr = addrScheme->myNetwAddr(netw);
        } else {
            myAppAddr = LAddress::L3Type( netw->getId() );
        }
        sentPackets = 0;

        // first packet generation time is always chosen uniformly
        // to avoid systematic collisions
        if(nbPackets> 0)
            scheduleAt(simTime() +uniform(initializationTime, initializationTime + trafficParam), delayTimer);

        if (stats) {
            latenciesRaw.setName("rawLatencies");
            latenciesRaw.setUnit("s");
            latency.setName("latency");
        }
    }
}

cStdDev& WhiteboxApplLayer::hostsLatency(const LAddress::L3Type& hostAddress)
{
    using std::pair;

    if(latencies.count(hostAddress) == 0) {
        std::ostringstream oss;
        oss << hostAddress;
        cStdDev aLatency(oss.str().c_str());
        latencies.insert(pair<LAddress::L3Type, cStdDev>(hostAddress, aLatency));
    }

    return latencies[hostAddress];
}

void WhiteboxApplLayer::initializeDistribution(const char* traffic) {
    if (!strcmp(traffic, "periodic")) {
        trafficType = PERIODIC;
    } else if (!strcmp(traffic, "uniform")) {
        trafficType = UNIFORM;
    } else if (!strcmp(traffic, "exponential")) {
        trafficType = EXPONENTIAL;
    } else {
        trafficType = UNKNOWN;
        EV << "Error! Unknown traffic type: " << traffic << endl;
    }
}

void WhiteboxApplLayer::scheduleNextPacket() {

    if(trafficType != 0){

        simtime_t waitTime = SIMTIME_ZERO;
        switch (trafficType) {
        case PERIODIC:
            waitTime = trafficParam;
            debugEV<< "Periodic traffic, waitTime=" << waitTime << endl;
            break;
            case UNIFORM:
            waitTime = uniform(0, trafficParam);
            debugEV << "Uniform traffic, waitTime=" << waitTime << endl;
            break;
            case EXPONENTIAL:
            waitTime = exponential(trafficParam);
            debugEV << "Exponential traffic, waitTime=" << waitTime << endl;
            break;
            case UNKNOWN:
            default:
            EV <<
            "Cannot generate requested traffic type (unimplemented or unknown)."
            << endl;
            return; // don not schedule
            break;
        }


        if ((nbPackets > sentPackets && trafficType != 0)) { // We must generate packets
                debugEV << "Start timer for a new packet in " << waitTime << " seconds." <<
                        endl;
                scheduleAt(simTime() + waitTime, delayTimer);
                debugEV << "...timer rescheduled." << endl;
        }else{
            debugEV << "All packets sent.\n";
            if(sleepAfterFinish){
                debugEV << "Start timer for turning the radio off in " << waitTime << " seconds." <<
                        endl;
                scheduleAt(simTime() + waitTime, turnOffTimer);
            }
        }
    }
}

/*
 * @brief overridden to use USB
 */
void WhiteboxApplLayer::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    } else if(msg->getArrivalGateId()==upperLayerIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::UPPER_DATA,msg);
        handleUpperMsg(msg);
    } else if(msg->getArrivalGateId()==upperControlIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::UPPER_CONTROL,msg);
        handleUpperControl(msg);
    } else if(msg->getArrivalGateId()==lowerControlIn){
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_CONTROL,msg);
        handleLowerControl(msg);
    } else if(msg->getArrivalGateId()==lowerLayerIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_DATA,msg);
        handleLowerMsg(msg);
    } else if(msg->getArrivalGateId()==USBLayerIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_DATA,msg);
        handleUSBMsg(msg);
    } else if(msg->getArrivalGateId()==-1) {
        /* Classes extending this class may not use all the gates, f.e.
         * BaseApplLayer has no upper gates. In this case all upper gate-
         * handles are initialized to -1. When getArrivalGateId() equals -1,
         * it would be wrong to forward the message to one of these gates,
         * as they actually don't exist, so raise an error instead.
         */
        opp_error("No self message and no gateID?? Check configuration.");
    } else {
        /* msg->getArrivalGateId() should be valid, but it isn't recognized
         * here. This could signal the case that this class is extended
         * with extra gates, but handleMessage() isn't overridden to
         * check for the new gate(s).
         */
        opp_error("Unknown gateID?? Check configuration or override handleMessage().");
    }
}


/**
 * @brief Handling of messages arrived to destination
 **/
void WhiteboxApplLayer::handleUSBMsg(cMessage * msg) {
    trafficParam = atof(msg->getName());
    delete msg;

    char value[100];

    incrementControlID();
    sprintf(value, "%d %lf", controlID, trafficParam);

    ApplPkt *pkt = new ApplPkt(value, UPDATE_PARAM);

    pkt->setDestAddr(LAddress::L3BROADCAST);
    pkt->setSrcAddr(myAppAddr);
    pkt->setByteLength(headerLength);
    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(pkt, pkt->getDestAddr());
    debugEV<< "Broadcasting update message, the new traffic parameter is " << value <<"!\n";
    sendDown(pkt);
    nbPacketsSent++;
    packet.setPacketSent(true);
    packet.setNbPacketsSent(1);
    packet.setNbPacketsReceived(0);
    packet.setHost(myAppAddr);
    emit(BaseLayer::catPacketSignal, &packet);
    sentPackets++;
}


/**
 * @brief Handling of messages arrived to destination
 **/
void WhiteboxApplLayer::handleLowerMsg(cMessage * msg) {
    ApplPkt *m;

    switch (msg->getKind()) {
    case DATA_MESSAGE:
        m = static_cast<ApplPkt *> (msg);
        nbPacketsReceived++;
        packet.setPacketSent(false);
        packet.setNbPacketsSent(0);
        packet.setNbPacketsReceived(1);
        packet.setHost(myAppAddr);
        emit(BaseLayer::catPacketSignal, &packet);
        if (stats) {
            simtime_t theLatency = m->getArrivalTime() - m->getCreationTime();
            if(trace) {
              hostsLatency(m->getSrcAddr()).collect(theLatency);
              latenciesRaw.record(SIMTIME_DBL(theLatency));
            }
            latency.collect(theLatency);
            if (firstPacketGeneration < 0)
                firstPacketGeneration = m->getCreationTime();
            lastPacketReception = m->getArrivalTime();
            if(trace) {
              debugEV<< "Received a data packet from host[" << m->getSrcAddr()
              << "], data=" << m->getName() << ", latency=" << theLatency
              << ", collected " << hostsLatency(m->getSrcAddr()).
              getCount() << "mean is now: " << hostsLatency(m->getSrcAddr()).
              getMean() << endl;
            } else {
                  debugEV<< "Received a data packet from host[" << m->getSrcAddr()
                  << "], data= " << m->getName() << " latency=" << theLatency << endl;
            }
        }

        sendUSBMsg(atoi(m->getName()));

        delete msg;
        break;
    case UPDATE_PARAM:

        m = static_cast<ApplPkt *> (msg);
        int controlIDTemp;
        double trafficParamTmp;
        sscanf(m->getName(), "%d %lf", &controlIDTemp, &trafficParamTmp);

        debugEV<< "Received a control packet from host[" << m->getSrcAddr()
                          << "], control id= " << controlIDTemp << " traffic param=" << trafficParamTmp << endl;

        if(controlIDTemp > controlID){
            controlID = controlIDTemp;
            updateTrafficParam(trafficParamTmp);
        }

        delete msg;
        break;
    default:
        EV << "Error! got packet with unknown kind: " << msg->getKind() << endl;
        delete msg;
        break;
    }
}

void WhiteboxApplLayer::sendUSBMsg(int receivedVal){

    char payload[10];
    sprintf(payload, "%d", receivedVal);

    ApplPkt *pkt = new ApplPkt(payload, UPDATE_PARAM);
    pkt->setDestAddr(LAddress::L3BROADCAST);
    pkt->setSrcAddr(myAppAddr);

    debugEV<< "Sending a packet to EG with value= " << payload << "." << endl;

    send(pkt, USBLayerOut);
}

void WhiteboxApplLayer::updateTrafficParam(double newVal){
    trafficParam = newVal;
//    cancelEvent(delayTimer);
//    scheduleNextPacket();

//    char payload[100];
//    sprintf(payload, "%d %lf", controlID, trafficParam);

//    ApplPkt *pkt = new ApplPkt(payload, UPDATE_PARAM);
//    pkt->setDestAddr(LAddress::L3BROADCAST);
//    pkt->setSrcAddr(myAppAddr);
//    debugEV<< "Transmitting a control packet with control id= " << controlID << " traffic param=" << trafficParam << endl;
//    sendDown(pkt);

}

int WhiteboxApplLayer::incrementControlID(){
    controlID++;
    return controlID;
}

/**
 * @brief A timer with kind = SEND_DATA_TIMER indicates that a new
 * data has to be send (@ref sendData).
 *
 * There are no other timers implemented for this module.
 *
 * @sa sendData
 **/
void WhiteboxApplLayer::handleSelfMsg(cMessage * msg) {
    switch (msg->getKind()) {
    case SEND_DATA_TIMER:
        scheduleNextPacket();
        sendData();
        //delete msg;
        break;
    case TURN_RADIO_OFF_TIMER:
        MacToPhyInterface* phy;
        phy = FindModule<MacToPhyInterface*>::findSubModule(findHost());
        phy->setRadioState(MiximRadio::SLEEP);
        break;
    default:
        EV<< "Unkown selfmessage! -> delete, kind: " << msg->getKind() << endl;
        delete msg;
        break;
    }
}

void WhiteboxApplLayer::handleLowerControl(cMessage * msg) {
    delete msg;
}

/**
  * @brief This function creates a new data message and sends it down to
  * the network layer
 **/
void WhiteboxApplLayer::sendData() {

    SensorPkt* sensorData = new SensorPkt("Sensor", SENSOR_DATA);
    sendUp(sensorData);
}


void WhiteboxApplLayer::handleUpperMsg(cMessage * msg) {

    SensorPkt* sensorData = static_cast<SensorPkt*>(msg);

    char value[4];
    sprintf(value, "%d", sensorData->getVal());

    ApplPkt *pkt = new ApplPkt(value, DATA_MESSAGE);

    if(broadcastPackets) {
        pkt->setDestAddr(LAddress::L3BROADCAST);
    } else {
        pkt->setDestAddr(destAddr);
    }
    pkt->setSrcAddr(myAppAddr);
    pkt->setByteLength(headerLength);
    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(pkt, pkt->getDestAddr());
    debugEV<< "Sending data packet, the temperature is " << sensorData->getVal() <<"!\n";
    delete msg;
    sendDown(pkt);
    nbPacketsSent++;
    packet.setPacketSent(true);
    packet.setNbPacketsSent(1);
    packet.setNbPacketsReceived(0);
    packet.setHost(myAppAddr);
    emit(BaseLayer::catPacketSignal, &packet);
    sentPackets++;
}

void WhiteboxApplLayer::finish() {
    using std::map;
    if (stats) {
        if (trace) {
            std::stringstream osToStr(std::stringstream::out);
            // output logs to scalar file
            for (map<LAddress::L3Type, cStdDev>::iterator it = latencies.begin(); it != latencies.end(); ++it) {
                cStdDev aLatency = it->second;

                osToStr.str(""); osToStr << "latency" << it->first;
                recordScalar(osToStr.str().c_str(), aLatency.getMean(), "s");
                aLatency.record();
            }
        }
        recordScalar("activity duration", lastPacketReception
                - firstPacketGeneration, "s");
        recordScalar("firstPacketGeneration", firstPacketGeneration, "s");
        recordScalar("lastPacketReception", lastPacketReception, "s");
        recordScalar("nbPacketsSent", nbPacketsSent);
        recordScalar("nbPacketsReceived", nbPacketsReceived);
        latency.record();
    }
    cComponent::finish();
}

WhiteboxApplLayer::~WhiteboxApplLayer() {
    cancelAndDelete(delayTimer);
    cancelAndDelete(turnOffTimer);
}
