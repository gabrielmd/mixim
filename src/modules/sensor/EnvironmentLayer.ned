package org.mixim.modules.sensor;

import org.mixim.base.modules.IBaseApplLayer;


simple EnvironmentLayer like IBaseApplLayer
{
    parameters:
        @class(EnvironmentLayer);
        bool   debug = default(false); // debug switch
        bool   stats = default(false);   // stats switch
        bool   trace = default(false); // trace switch

        // random length
        int headerLength @unit(bit) = default(8 bit);
        
        string filename = default("");
    gates:

        input lowerLayerIn; // from sensors
        input lowerControlIn;
        output lowerLayerOut; // to sensors
        output lowerControlOut;

       // input upperLayerIn; // from application layer
       // output upperLayerOut; // to application layer
       // input upperControlIn; // control from application layer
       // output upperControlOut; // control to application layer
}
