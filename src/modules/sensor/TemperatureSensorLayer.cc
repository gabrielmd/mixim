/***************************************************************************
 * file:        TemperatureSensorLayer.h
 **************************************************************************/

#include "TemperatureSensorLayer.h"

#include "BaseNetwLayer.h"
#include "AddressingInterface.h"
#include "NetwControlInfo.h"
#include "FindModule.h"
#include "SimpleAddress.h"
#include "MacToPhyInterface.h"
#include "BaseWorldUtility.h"

#include "BaseMacLayer.h"
#include "NetwControlInfo.h"
#include "SensorPkt_m.h"

Define_Module(TemperatureSensorLayer);

void TemperatureSensorLayer::initialize(int stage) {
    BaseLayer::initialize(stage);

    if (stage == 0) {

        // Retrieve parameters
        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        trace = par("trace").boolValue();

        sleepingCurrent = par("sleepingCurrent");
        measuringCurrent = par("measuringCurrent");
        measuringTime = par("measuringTime");

        measure = new cMessage("measure", MEASUREMENT);

        // statistics
        numMeasurementsRequests = 0;
        numMeasurementsDone = 0;
    }else if(stage == 1){
        registerWithBattery("TemperatureSensorLayer", 1);
        drawCurrent(sleepingCurrent, 0);
    }

}

TemperatureSensorLayer::~TemperatureSensorLayer() {
    cancelAndDelete(measure);
}

void TemperatureSensorLayer::finish() {
    if(stats){
        recordScalar("nr of measurements requested", numMeasurementsRequests);
        recordScalar("nr of measurements done", numMeasurementsDone);
    }

    cComponent::finish();
}

/**
 * @brief handles the request to make a measurement.
 */
void TemperatureSensorLayer::handleLowerMsg(cMessage * msg) {

    numMeasurementsRequests++;
    measure->setContextPointer(msg);
    drawCurrent(measuringCurrent, 0);
    scheduleAt(simTime() + measuringTime, measure);
}

/**
 * @brief handles self messages.
 */
void TemperatureSensorLayer::handleSelfMsg(cMessage * msg){
    if (msg->isSelfMessage()) {
        EV<< simTime() << " " << msg->getName() << endl;

        switch (msg->getKind()) {
            case MEASUREMENT:
            {
                /*
                 * After simulating the time spent for the measurement, it sends
                 * a message to the upper layer (the environment) and gets the
                 * real data.
                 */
                SensorPkt* pkt = static_cast<SensorPkt*>(msg->getContextPointer());
                sendUp(pkt);
                break;
            }
            default:
                error("unknown selfMsg");
                break;
        }
    }
}

/*
 * @brief handles the real value.
 */
void TemperatureSensorLayer::handleUpperMsg(cMessage * msg) {
    SensorPkt* pkt = static_cast<SensorPkt*>(msg);
    debugEV<< "Sending temperature value: " << pkt->getVal() << "\n";
    drawCurrent(sleepingCurrent, 0);
    sendDown(pkt);
    numMeasurementsDone++;
}
