/***************************************************************************
 * file:        EnvironmentLayer.h
 **************************************************************************/

#include "EnvironmentLayer.h"

#include <sstream>

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

Define_Module(EnvironmentLayer);

void EnvironmentLayer::initialize(int stage) {
    char line[100];
    char c;

    BaseLayer::initialize(stage);


    if (stage == 0) {

        // Retrieve parameters
        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        trace = par("trace").boolValue();

        filename = par("filename").stringValue();

        totalIntervals = 0;

        openSourceFile(filename);

        while (!_inputFileStream.eof())
        {
            // second line: initial delay unit description
            _inputFileStream.get(line, 100, '\n');
            sscanf(line, "%lf %d", &endTimes[totalIntervals], &temperatures[totalIntervals]);
            _inputFileStream.get(c); // new line char
            debugEV << line << endl;
            totalIntervals++;
        }

        closeSourceFile();
    }
}

void EnvironmentLayer::openSourceFile(const char *fileName)
{
    _inputFileStream.open(fileName);
    if (!_inputFileStream)
        throw cRuntimeError("Error opening data file '%s'", fileName);
}

void EnvironmentLayer::closeSourceFile()
{
    _inputFileStream.close();
}


EnvironmentLayer::~EnvironmentLayer() {
    return;
}

void EnvironmentLayer::finish() {
    cComponent::finish();
}

void EnvironmentLayer::handleLowerMsg(cMessage * msg) {

    SensorPkt* pkt = static_cast<SensorPkt*>(msg);

    int i;
    for(i=0; i < totalIntervals; i++){
        if(SIMTIME_DBL(simTime()) < endTimes[i]){
            pkt->setVal(temperatures[i]);
            sendDown(pkt);
            return;
        }

    }

    error("did not find the temperature");
}
