package org.mixim.modules.sensor;

import org.mixim.base.modules.IBaseNetwLayer;


simple TemperatureSensorLayer like IBaseNetwLayer
{
    parameters:
        @class(TemperatureSensorLayer);
        bool debug = default(false); // debug switch
        bool stats = default(false);   // stats switch
        bool trace = default(false); // trace switch

        // from datasheet
        int headerLength @unit(bit) = default(12 bit);

        // energy consumption
        double sleepingCurrent @unit(mA) = default(0.0003 mA);
        double measuringCurrent @unit(mA) = default(0.55 mA);
        double measuringTime @unit(s) = default(0.08 s);
    gates:

        input lowerLayerIn; // from network layer
        input lowerControlIn;
        output lowerLayerOut; // to network layer
        output lowerControlOut;

        input upperLayerIn; // from environment
        output upperLayerOut; // to environment
        input upperControlIn; // control from environment
        output upperControlOut; // control to environment
}
