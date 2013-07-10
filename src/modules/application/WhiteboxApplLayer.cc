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

        debugEV<< "in initialize() stage 0...";

        // USB connection
        USBLayerIn  = findGate("USBLayerIn");
        USBLayerOut = findGate("USBLayerOut");
        USBControlIn  = findGate("USBControlIn");
        USBControlOut = findGate("USBControlOut");

        // general configuration
        debug = par("debug");
        stats = par("stats");
        trace = par("trace");
        nbPackets = par("nbPackets");
        trafficParam = par("trafficParam");
        initializationTime = par("initializationTime");
        headerLength = par("headerLength");

        // application configuration
        const char *traffic = par("trafficType");
        destAddr = LAddress::L3Type(par("destAddr").longValue());
        nbPacketsSent = 0;
        nbPacketsReceived = 0;
        nbPacketsSentBroadcast = 0;
        nbPacketsReceivedBroadcast = 0;
        nbPacketsSentUSB = 0;
        nbPacketsReceivedUSB = 0;
        firstPacketGeneration = -1;
        lastPacketReception = -2;
        sleepAfterFinish = par("sleepAfterFinish").boolValue();
        initializeDistribution(traffic);

        // internal control
        controlID = 0;

        // timers: delay to next message, time to turn the node off
        delayTimer = new cMessage("appDelay", SEND_DATA_TIMER);
        turnOffTimer = new cMessage("appDelay", TURN_RADIO_OFF_TIMER);


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
        sentPacketsSensors = 0;

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

    MacToPhyInterface* phy;

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
            return; // do not schedule
            break;
        }


        if ((nbPackets > sentPacketsSensors && trafficType != 0)) { // We must generate packets
            debugEV << "Start timer for a new packet in " << waitTime << " seconds." << endl;
            scheduleAt(simTime() + waitTime, delayTimer);
            debugEV << "...timer rescheduled." << endl;
        }else{
            debugEV << "All packets sent.\n";
            phy = FindModule<MacToPhyInterface*>::findSubModule(findHost());
            if(sleepAfterFinish && (phy->getRadioState() != MiximRadio::SLEEP)){
                debugEV << "Start timer for turning the radio off in " << waitTime << " seconds." << endl;
                scheduleAt(simTime() + waitTime, turnOffTimer);
                debugEV << "...timer rescheduled." << endl;
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
    } else if(msg->getArrivalGateId()==lowerLayerIn) { // from wireless connection
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_DATA,msg);
        handleLowerMsg(msg);
    } else if(msg->getArrivalGateId()==USBLayerIn) { // from usb
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
 * @brief Handling of messages arrived at the USB port
 **/
void WhiteboxApplLayer::handleUSBMsg(cMessage * msg) {

    /* Begin stats */
    nbPacketsReceivedUSB++;
    packet.setPacketSent(false);
    packet.setNbPacketsSent(0);
    packet.setNbPacketsReceived(1);
    packet.setHost(myAppAddr);
    emit(BaseLayer::catPacketSignal, &packet);
    simtime_t theLatency = msg->getArrivalTime() - msg->getCreationTime();
    debugEV<< "Received a data packet via USB, data= " << msg->getName() << " latency=" << theLatency << endl;
    /* Finish stats */

    // reads the message
    double trafficParamMsg = atof(msg->getName());

    // updates the system
    incrementControlID();
    updateTrafficParam(trafficParamMsg);

    /* Start broadcasting */
    // updates the network -> broadcasts a message
    char value[100];
    sprintf(value, "%d %lf", controlID, trafficParam); // builds the message

    ApplPkt *pkt = new ApplPkt(value, UPDATE_PARAM);
    pkt->setDestAddr(LAddress::L3BROADCAST);
    pkt->setSrcAddr(myAppAddr);
    pkt->setByteLength(headerLength);
    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(pkt, LAddress::L3BROADCAST);
    debugEV<< "Broadcasting update message, the new traffic parameter is " << value <<"!\n";
    sendDown(pkt);
    /* Finish broadcasting */

    nbPacketsSentBroadcast++;
    packet.setPacketSent(true);
    packet.setNbPacketsSent(1);
    packet.setNbPacketsReceived(0);
    packet.setHost(myAppAddr);
    emit(BaseLayer::catPacketSignal, &packet);

    // removes the message
    delete msg;
}


void WhiteboxApplLayer::computeStatisticsNbPacketReceived(ApplPkt * m)
{
    /* Begin statistics */
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
}

/**
 * @brief Handling of messages arrived to destination
 **/
void WhiteboxApplLayer::handleLowerMsg(cMessage * msg) {

    ApplPkt *m;

    switch (msg->getKind()) {

    case DATA_MESSAGE: // measured value

        m = static_cast<ApplPkt *> (msg);

        computeStatisticsNbPacketReceived(m); // statistics

        /*
         * This is the sink node, otherwise the forwarded packets would not achieve
         * the application layer.
         * So, there is a connection between this node and the Enhanced Gateway, which
         * is done via USB. It is going to transmit the received value to it.
         */
        sendUSBMsg(atoi(m->getName()));

        delete msg;
        break;
    case UPDATE_PARAM: // received a new time interval between two measurements/tranmissions

        m = static_cast<ApplPkt *> (msg);

        computeStatisticsNbPacketReceived(m); // statistics

        // reads the message and checks the values
        int controlIDTemp;
        double trafficParamTmp;
        sscanf(m->getName(), "%d %lf", &controlIDTemp, &trafficParamTmp);

        /*
         * Be sure that the received message is not old.
         * If the network is too big, some updates might
         * take longer.
         */
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

/**
 * @brief Transmits the data to the EG via USB.
 */
void WhiteboxApplLayer::sendUSBMsg(int receivedVal){

    char payload[10];
    sprintf(payload, "%d", receivedVal);

    ApplPkt *pkt = new ApplPkt(payload, UPDATE_PARAM);
    pkt->setDestAddr(LAddress::L3BROADCAST);
    pkt->setSrcAddr(myAppAddr);
    debugEV<< "Sending a packet to EG with value= " << payload << "." << endl;
    send(pkt, USBLayerOut);
}

/*
 * @brief Handles the received parameter.
 */
void WhiteboxApplLayer::updateTrafficParam(double newVal){
    trafficParam = newVal;
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
 * @brief This function creates a new data message and sends it up to
 * the sensor layer. It will fire the timer at the sensor, simulating
 * a measurement and the value can be transmitted later.
 **/
void WhiteboxApplLayer::sendData() {

    SensorPkt* sensorData = new SensorPkt("Sensor", SENSOR_DATA);
    sendUp(sensorData);
}

/**
 * @brief This function gets the measured value transmitted by the
 * sensor (upper) layer. Now it can transmit the data to the
 * network.
 */
void WhiteboxApplLayer::handleUpperMsg(cMessage * msg) {

    SensorPkt* sensorData = static_cast<SensorPkt*>(msg);

    char value[4];
    sprintf(value, "%d", sensorData->getVal());

    ApplPkt *pkt = new ApplPkt(value, DATA_MESSAGE);
    pkt->setDestAddr(destAddr);
    pkt->setSrcAddr(myAppAddr);
    pkt->setByteLength(headerLength);

    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(pkt, pkt->getDestAddr());
    debugEV<< "Sending data packet, the temperature is " << sensorData->getVal() <<"!\n";
    delete msg;
    sendDown(pkt);

    /* Begin statistics */
    nbPacketsSent++;
    packet.setPacketSent(true);
    packet.setNbPacketsSent(1);
    packet.setNbPacketsReceived(0);
    packet.setHost(myAppAddr);
    emit(BaseLayer::catPacketSignal, &packet);
    sentPacketsSensors++;
    /* Finish statistics */
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
        recordScalar("activity duration", lastPacketReception - firstPacketGeneration, "s");
        recordScalar("firstPacketGeneration", firstPacketGeneration, "s");
        recordScalar("lastPacketReception", lastPacketReception, "s");
        recordScalar("nbPacketsSent", nbPacketsSent);
        recordScalar("nbPacketsReceived", nbPacketsReceived);
        recordScalar("nbPacketsSentBroadcast", nbPacketsSentBroadcast);
        recordScalar("nbPacketsReceivedBroadcast", nbPacketsReceivedBroadcast);
        recordScalar("nbPacketsSentUSB", nbPacketsSentUSB);
        recordScalar("nbPacketsReceivedUSB", nbPacketsReceivedUSB);
        latency.record();
    }
    cComponent::finish();
}

WhiteboxApplLayer::~WhiteboxApplLayer() {
    cancelAndDelete(delayTimer);
    cancelAndDelete(turnOffTimer);
}
