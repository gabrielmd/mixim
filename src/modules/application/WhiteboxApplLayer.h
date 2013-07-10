/***************************************************************************
 * file:        WhiteboxApplLayer.h
 **************************************************************************/

#ifndef WHITEBOX_APPL_LAYER_H
#define WHITEBOX_APPL_LAYER_H

#include <omnetpp.h>

#include "ApplPkt_m.h"
#include "SensorPkt_m.h"
#include "BaseSensorApplLayer.h"
#include "BaseWorldUtility.h"
#include "Packet.h"

/**
 **/
class WhiteboxApplLayer : public BaseSensorApplLayer
{
private:
    WhiteboxApplLayer(const WhiteboxApplLayer& );
    WhiteboxApplLayer & operator =(const WhiteboxApplLayer& );
    void computeStatisticsNbPacketReceived(ApplPkt * m);
    int controlID;
    int sentPacketsSensors;
public:
    virtual void initialize(int);
    virtual void finish();
    virtual ~WhiteboxApplLayer();
    WhiteboxApplLayer()
    :BaseSensorApplLayer(), delayTimer(NULL), myAppAddr(), destAddr(), initializationTime(), firstPacketGeneration(), lastPacketReception(), trafficType(0), nbPackets(0), nbPacketsSent(0), nbPacketsReceived(0), nbPacketsSentBroadcast(0), nbPacketsReceivedBroadcast(0), nbPacketsSentUSB(0), nbPacketsReceivedUSB(0), stats(false), trace(false), debug(false), latencies(), latency(), latenciesRaw(), packet(100), headerLengthUSB(0), headerLengthUDP(0), dataOut(0), dataIn(0), ctrlOut(0), ctrlIn(0), USBLayerIn(-1), USBLayerOut(-1), USBControlIn(-1), USBControlOut(-1), trafficParam(0.0), sentPacketsSensors(0)
    {
    }

    enum APPL_MSG_TYPES{ SEND_DATA_TIMER, STOP_SIM_TIMER, DATA_MESSAGE, TURN_RADIO_OFF_TIMER, SENSOR_DATA, UPDATE_PARAM = 7919};
    enum TRAFFIC_TYPES{ UNKNOWN = 0, PERIODIC, UNIFORM, EXPONENTIAL, NB_DISTRIBUTIONS};
protected:
    cMessage *delayTimer;
    cMessage *turnOffTimer;
    LAddress::L3Type myAppAddr;
    LAddress::L3Type destAddr;
    simtime_t initializationTime;
    simtime_t firstPacketGeneration;
    simtime_t lastPacketReception;
    int trafficType;
    int nbPackets;
    bool sleepAfterFinish;
    long nbPacketsSent;
    long nbPacketsReceived;
    long nbPacketsSentBroadcast;
    long nbPacketsReceivedBroadcast;
    long nbPacketsSentUSB;
    long nbPacketsReceivedUSB;
    bool stats;
    bool trace;
    bool debug;
    std::map<LAddress::L3Type,cStdDev> latencies;
    cStdDev latency;
    cOutVector latenciesRaw;
    Packet packet;
    int headerLengthUSB;
    int headerLengthUDP;
protected:
    int dataOut;
    int dataIn;
    int ctrlOut;
    int ctrlIn;
    int USBLayerIn;
    int USBLayerOut;
    int USBControlIn;
    int USBControlOut;
    virtual void handleMessage(cMessage*);
    virtual void handleSelfMsg(cMessage*);
    virtual void handleLowerMsg(cMessage*);
    virtual void updateTrafficParam(double);
    virtual int incrementControlID();
    virtual void handleUSBMsg(cMessage*);
    virtual void handleLowerControl(cMessage *msg);
    virtual void handleUpperMsg(cMessage *msg);
    virtual void handleUpperControl(cMessage *m)
    {
        delete m;
    }

    virtual void sendData();
    virtual void initializeDistribution(const char*);
    virtual void scheduleNextPacket();
    virtual void sendUSBMsg(int);
    cStdDev& hostsLatency(const LAddress::L3Type& hostAddress);

public:
    double trafficParam;
};

#endif
