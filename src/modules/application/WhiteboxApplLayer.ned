package org.mixim.modules.application;

import org.mixim.base.modules.IBaseSensorApplLayer;


simple WhiteboxApplLayer like IBaseSensorApplLayer
{
    parameters:
        bool debug = default(false); // debug switch
        bool stats = default(false);   // stats switch
        bool trace = default(false); // trace switch

        bool broadcastPackets = default(false); // send packets in broadcast mode
        double nbPackets = default(0);
        int destAddr = default(0);
        string trafficType = default("periodic");  	// Can be one of: periodic, uniform or exponential
        double trafficParam @unit(s) = default(1 s);			// the mean time between two packets
        double initializationTime @unit(s) = default(1 min); // minimum time before generation of the first packet
        int headerLength @unit(byte) = default(2 byte);
        bool notAffectedByHostState = default(true);
        bool sleepAfterFinish;

    gates:
        input lowerLayerIn; // from network layer
        input lowerControlIn;
        output lowerLayerOut; // to network layer
        output lowerControlOut;

        input upperLayerIn; // from sensor layer
        output upperLayerOut; // to sensor layer
        input upperControlIn; // control from sensor layer
        output upperControlOut; // control to sensor layer

        input USBLayerIn; // from usb
        output USBLayerOut; // to usb
        input USBControlIn; // control from usb
        output USBControlOut; // control to usb


}
