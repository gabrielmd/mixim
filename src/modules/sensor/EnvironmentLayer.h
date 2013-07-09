/***************************************************************************
 * file:        EnvironmentLayer.h
 **************************************************************************/

#ifndef ENVIRONMENT_LAYER_H
#define ENVIRONMENT_LAYER_H

#include <map>
#include <fstream>

#include "MiXiMDefs.h"
#include "BaseModule.h"
#include "BaseLayer.h"
#include "Packet.h"
#include "SimpleAddress.h"
#include "BaseWorldUtility.h"

class BaseWorldUtility;

class MIXIM_API EnvironmentLayer : public BaseLayer {

private:
    /** @brief Copy constructor is not allowed.
     */
    EnvironmentLayer(const EnvironmentLayer&);
    /** @brief Assignment operator is not allowed.
     */
    EnvironmentLayer& operator=(const EnvironmentLayer&);

    virtual void openSourceFile(const char *);
    virtual void closeSourceFile();

public:
    EnvironmentLayer()
        : BaseLayer()
        , debug(false), stats(false), trace(false)
        , totalIntervals(0)
    {}

    virtual ~EnvironmentLayer();

    virtual void initialize(int stage);
    virtual void finish();

protected:

        // module parameters
        bool debug, stats, trace;

        // scenario vars
        double endTimes[1000];
        int temperatures[1000];
        int totalIntervals;

        const char *filename;

        /**
         * The input file stream for the data file.
         */
        std::ifstream  _inputFileStream;

        virtual void handleSelfMsg(cMessage * m)
        {
            delete m;
        }

        virtual void handleLowerMsg(cMessage *);
        virtual void handleUpperMsg(cMessage * m)
        {
            delete m;
        }

        virtual void handleUpperControl(cMessage * m)
        {
            delete m;
        }

        virtual void handleLowerControl(cMessage * m){
            delete m;
        }
};
#endif
