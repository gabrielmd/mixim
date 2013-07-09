/***************************************************************************
 * file:       StaticRoute.cc
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Adaptor module that simply "translates" netwControlInfo to macControlInfo
 *
 **************************************************************************/
#include "StaticRoute.h"

#include <limits>
#include <algorithm>
#include <cassert>

#include "WiseRoutePkt_m.h"
#include "NetwControlInfo.h"
#include "ArpInterface.h"
#include "NetwPkt_m.h"

using std::endl;

Define_Module(StaticRoute);

void StaticRoute::initialize(int stage) {
	BaseNetwLayer::initialize(stage);
	if (stage == 1) {
		trace = par("trace");
		stats = par("stats");
		//networkID = par("networkID");

		nbDataPacketsForwarded = 0;
		nbDataPacketsReceived = 0;
		nbDataPacketsSent = 0;
	}
}


void StaticRoute::handleLowerMsg(cMessage* msg) {

    // old code
    // WiseRoutePkt* pkt = check_and_cast<WiseRoutePkt*>(msg);
	//if(pkt->getNetworkID()==networkID) {
	//	sendUp(decapsMsg(pkt));
	//} else {
	//	delete pkt;
	//}

    // new code

    WiseRoutePkt*           netwMsg        = check_and_cast<WiseRoutePkt*>(msg);
    const LAddress::L3Type& finalDestAddr  = netwMsg->getFinalDestAddr();

    if (finalDestAddr == myNetwAddr || LAddress::isL3Broadcast(finalDestAddr)) {

        debugEV << "StaticRoute received a packet to this address from " << netwMsg->getSrcAddr() << "\n";

        WiseRoutePkt* msgCopy;
        msgCopy = netwMsg;
        if (msgCopy->getKind() == DATA) {
            nbDataPacketsReceived++;
            sendUp(decapsMsg(msgCopy));
        }
        else {
            delete msgCopy;
        }
    } else {

        if (netwMsg->getKind() == DATA) {
            nbDataPacketsForwarded++;
        }
        debugEV << "StaticRoute received a packet to be forwarded from " << netwMsg->getSrcAddr() << " to " << netwMsg->getFinalDestAddr() << "\n";

        const cObject* pCtrlInfo = NULL;
        LAddress::L3Type nextHop = myNetwAddr - 1;
        if (LAddress::isL3Broadcast(nextHop)) {
            // no route exist to destination, attempt to send to final destination
            nextHop = finalDestAddr;
        }

        pCtrlInfo = netwMsg->removeControlInfo();
        if (pCtrlInfo != NULL){
            delete pCtrlInfo;
        }

        debugEV << "StaticRoute next hop is " << nextHop << " with mac " << arp->getMacAddr(nextHop) << "\n";

        netwMsg->setSrcAddr(myNetwAddr);
        netwMsg->setDestAddr(nextHop);
        setDownControlInfo(netwMsg, arp->getMacAddr(nextHop));
        netwMsg->setNbHops(netwMsg->getNbHops()+1);
        sendDown(netwMsg);
    }
}

void StaticRoute::handleLowerControl(cMessage *msg) {
    delete msg;
}

void StaticRoute::handleUpperMsg(cMessage* msg) {
//	NetwControlInfo* cInfo =
//			dynamic_cast<NetwControlInfo*> (msg->removeControlInfo());
//	LAddress::L2Type nextHopMacAddr;
//	if (cInfo == 0) {
//		EV<<"DummyRoute warning: Application layer did not specifiy a destination L3 address\n"
//	       << "\tusing broadcast address instead\n";
//		nextHopMacAddr = LAddress::L2BROADCAST;
//	} else {
//		nextHopMacAddr = arp->getMacAddr(cInfo->getNetwAddr());
//	}
//	LAddress::setL3ToL2ControlInfo(msg, myNetwAddr, nextHopMacAddr);

    // old code
    //sendDown(encapsMsg(check_and_cast<cPacket*>(msg)));

    // new code
    LAddress::L3Type finalDestAddr;
    LAddress::L3Type nextHopAddr;
    LAddress::L2Type nextHopMacAddr;
    WiseRoutePkt*    pkt   = new WiseRoutePkt(msg->getName(), DATA);
    cObject*         cInfo = msg->removeControlInfo();

    pkt->setByteLength(headerLength);

    if ( cInfo == NULL ) {
        EV << "StaticRoute warning: Application layer did not specifiy a destination L3 address\n"
           << "\tusing broadcast address instead\n";
        finalDestAddr = LAddress::L3BROADCAST;
    }
    else {
        EV <<"StaticRoute: CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) <<endl;
        finalDestAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
        delete cInfo;
    }

    pkt->setFinalDestAddr(finalDestAddr);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setNbHops(0);

    if (LAddress::isL3Broadcast(finalDestAddr))
        nextHopAddr = LAddress::L3BROADCAST;
    else if(myNetwAddr > 0){
        nextHopAddr = myNetwAddr - 1;
    }


    pkt->setIsFlood(0);
    nextHopMacAddr = arp->getMacAddr(nextHopAddr);

    setDownControlInfo(pkt, nextHopMacAddr);
    assert(static_cast<cPacket*>(msg));
    pkt->encapsulate(static_cast<cPacket*>(msg));
    sendDown(pkt);
    nbDataPacketsSent++;
}

void StaticRoute::finish() {
    if (stats) {
            recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
            recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
            recordScalar("nbDataPacketsSent", nbDataPacketsSent);

            nbDataPacketsForwardedVector.setName("nbDataPacketsForwarded");
            nbDataPacketsForwardedVector.record(nbDataPacketsForwarded);

            nbDataPacketsReceivedVector.setName("nbDataPacketsReceived");
            nbDataPacketsReceivedVector.record(nbDataPacketsReceived);

            nbDataPacketsSentVector.setName("nbDataPacketsSent");
            nbDataPacketsSentVector.record(nbDataPacketsSent);
    }
    BaseNetwLayer::finish();
}
//
//StaticRoute::netwpkt_ptr_t StaticRoute::encapsMsg(cPacket *appPkt) {
//    LAddress::L2Type macAddr;
//    LAddress::L3Type netwAddr;
//
//    debugEV <<"in encaps...\n";
//
//    WiseRoutePkt *pkt = new WiseRoutePkt(appPkt->getName(), appPkt->getKind());
//    pkt->setBitLength(headerLength);
//
//    cObject* cInfo = appPkt->removeControlInfo();
//
//    if(cInfo == NULL){
//	  EV << "warning: Application layer did not specifiy a destination L3 address\n"
//	   << "\tusing broadcast address instead\n";
//	  netwAddr = LAddress::L3BROADCAST;
//    } else {
//	  debugEV <<"CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) << endl;
//	  netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
//	  delete cInfo;
//    }
//
//    pkt->setSrcAddr(myNetwAddr);
//    pkt->setDestAddr(netwAddr);
//    debugEV << " netw "<< myNetwAddr << " sending packet" <<endl;
//    if(LAddress::isL3Broadcast(netwAddr)) {
//        debugEV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
//           << " -> set destMac=L2BROADCAST\n";
//        macAddr = LAddress::L2BROADCAST;
//    }
//    else{
//        debugEV <<"sendDown: get the MAC address\n";
//        macAddr = arp->getMacAddr(netwAddr);
//    }
//
//    setDownControlInfo(pkt, macAddr);
//
//    //encapsulate the application packet
//    pkt->encapsulate(appPkt);
//    debugEV <<" pkt encapsulated\n";
//    return pkt;
//}

cPacket* StaticRoute::decapsMsg(netwpkt_ptr_t msg) {
	cPacket *m = msg->decapsulate();
	setUpControlInfo(m, msg->getSrcAddr());
		// delete the netw packet
	delete msg;
	return m;
}
