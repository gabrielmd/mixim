package org.mixim.base.modules;

// Interface for network layer modules
moduleinterface IBaseSensorApplLayer
{
    parameters:
        bool stats; // stats switch
        int  headerLength @unit("bit"); // length of the network packet header (in bits)
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

