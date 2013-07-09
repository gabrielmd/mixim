package org.mixim.examples.mynetwork;

import org.mixim.base.modules.BaseNetwork;
import org.mixim.modules.node.Host802154_2400MHz;


//
//This example shows the basic structure of an network using MiXiM.
//
//The network consists of three hosts which use the CSMAMacLayer as MAC layer,
//"SimplePathlossModel" as only AnalogueModel and a SNRThresholdDecider as Decider.
//
//The used Network- and Application layer can be defined in the omnetpp.ini. As default
//the BaseNetwLayer and BurstApplLayer are used.
//
//You can use this network as an template for you own simulation.
//
network MyNetwork extends NetworkOne
{
    parameters:
   //     int numNodesInNetworkOne; // total number of weak hosts in the network
     //   int numNodesInNetworkTwo;
        // @class(mynetwork); // TODO continue...
 		// networkOne = NetworkOne;
	submodules:
       // NetOne: NetworkOne {
       //     parameters:
        //        numNodes = numNodesInNetworkOne;
        //}
//        NetTwo: NetworkOne {
//            parameters:
//				numNodes = numNodesInNetworkTwo;
//        }
}

