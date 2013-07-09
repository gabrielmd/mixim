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
class MIXIM_API WhiteboxApplLayer : public BaseSensorApplLayer
{

private:
    /** @brief Copy constructor is not allowed.
     */
    WhiteboxApplLayer(const WhiteboxApplLayer&);
    /** @brief Assignment operator is not allowed.
     */
    WhiteboxApplLayer& operator=(const WhiteboxApplLayer&);

public:

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~WhiteboxApplLayer();

    WhiteboxApplLayer() :
        BaseSensorApplLayer(), delayTimer(NULL), myAppAddr(), destAddr(), sentPackets(0), initializationTime(), firstPacketGeneration(), lastPacketReception(), trafficType(
                    0), nbPackets(0), nbPacketsSent(0), nbPacketsReceived(0), stats(false), trace(
                    false), debug(false), broadcastPackets(false), latencies(), latency(), latenciesRaw(), packet(
                    100), headerLength(0), world(NULL), dataOut(0), dataIn(0), ctrlOut(0), ctrlIn(0),
                    USBLayerIn(-1), USBLayerOut(-1), USBControlIn(-1), USBControlOut(-1)
                    , trafficParam(0.0)
    {
    } // we must specify a packet length for Packet.h

    enum APPL_MSG_TYPES
    {
        SEND_DATA_TIMER, STOP_SIM_TIMER, DATA_MESSAGE, TURN_RADIO_OFF_TIMER, SENSOR_DATA, UPDATE_PARAM = 7919
    };

    enum TRAFFIC_TYPES
    {
        UNKNOWN = 0, PERIODIC, UNIFORM, EXPONENTIAL, NB_DISTRIBUTIONS,
    };

protected:
    cMessage * delayTimer;
    cMessage * turnOffTimer;

    LAddress::L3Type myAppAddr;
    LAddress::L3Type destAddr;
    int sentPackets;
    simtime_t initializationTime;
    simtime_t firstPacketGeneration;
    simtime_t lastPacketReception;
    // parameters:
    int trafficType;
    int nbPackets;
    long nbPacketsSent;
    long nbPacketsReceived;
    bool stats;
    bool trace;
    bool debug;
    bool broadcastPackets;

    bool sleepAfterFinish;

    std::map<LAddress::L3Type, cStdDev> latencies;
    cStdDev latency;
    cOutVector latenciesRaw;
    Packet packet; // informs the simulation of the number of packets sent and received by this node.
    int headerLength;
    BaseWorldUtility* world;

    int controlID;

protected:
    // gates
    int dataOut;
    int dataIn;
    int ctrlOut;
    int ctrlIn;

    // usb connection
    int USBLayerIn;
    int USBLayerOut;
    int USBControlIn;
    int USBControlOut;

    /*
     * @brief overridden to use USB
     */
    virtual void handleMessage(cMessage*);

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage *);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *);

    /** @brief update traffic param in network */
    virtual void updateTrafficParam(double);

    /** @brief update control counter */
    virtual int incrementControlID();

    /** @brief Handle messages from USB layer */
    virtual void handleUSBMsg(cMessage *);

    virtual void handleLowerControl(cMessage * msg);

    virtual void handleUpperMsg(cMessage * msg);

    virtual void handleUpperControl(cMessage * m)
    {
        delete m;
    }

    /** @brief send a data packet to the next hop */
    virtual void sendData();

    /** @brief Recognize distribution name. Redefine this method to add your own distribution. */
    virtual void initializeDistribution(const char*);

    /** @brief calculate time to wait before sending next packet, if required. You can redefine this method in a subclass to add your own distribution. */
    virtual void scheduleNextPacket();

    virtual void sendUSBMsg(int);

    /**
     * @brief Returns the latency statistics variable for the passed host address.
     * @param hostAddress the address of the host to return the statistics for.
     * @return A reference to the hosts latency statistics.
     */
    cStdDev& hostsLatency(const LAddress::L3Type& hostAddress);

public:
    double trafficParam;
};

#endif
