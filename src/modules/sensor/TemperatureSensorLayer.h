/***************************************************************************
 * file:        TemperatureSensorLayer.h
 **************************************************************************/

#ifndef TEMPERATURE_SENSOR_LAYER_H
#define TEMPERATURE_SENSOR_LAYER_H

#include <map>

#include "MiXiMDefs.h"
#include "BaseModule.h"
#include "BaseLayer.h"
#include "Packet.h"
#include "SimpleAddress.h"
#include "BaseWorldUtility.h"

class BaseWorldUtility;

class MIXIM_API TemperatureSensorLayer : public BaseLayer {

private:
    /** @brief Copy constructor is not allowed.
     */
    TemperatureSensorLayer(const TemperatureSensorLayer&);
    /** @brief Assignment operator is not allowed.
     */
    TemperatureSensorLayer& operator=(const TemperatureSensorLayer&);

public:
    TemperatureSensorLayer()
        : BaseLayer()
        //, dataOut(-1)
        //, dataIn(-1)
        //, ctrlOut(-1)
        //, ctrlIn(-1)
        , debug(false), stats(false), trace(false)
    {}

    enum APPL_MSG_TYPES
    {
        DATA_MESSAGE
    };

    virtual ~TemperatureSensorLayer();

    virtual void initialize(int stage);
    virtual void finish();

protected:

        // gates
        //int dataOut;
        //int dataIn;
        //int ctrlOut;
        //int ctrlIn;

        // module parameters
        bool debug, stats, trace;
        double sleepingCurrent, measuringCurrent, measuringTime;

        virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleUpperMsg(cMessage *);
        virtual void handleUpperControl(cMessage * m)
        {
            delete m;
        }

        virtual void handleLowerControl(cMessage * m){
            delete m;
        }
        enum DeviceState {
          MEASUREMENT
        };

        cMessage *measure;
};
#endif
