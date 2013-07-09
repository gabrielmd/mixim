package org.mixim.modules.node;

import inet.applications.IUDPUSBApp;
import inet.linklayer.IWiredNic;
import inet.nodes.inet.NodeBase;
import inet.transport.IUDP;


// check StandardHost
module EG extends NodeBase
{
    parameters:
        @display("i=device/pc2");

        string udpType = default(firstAvailable("UDP","UDP_None"));
        string usbType = default("");

    gates:
        inout usbg;

    submodules:
        udpApp: <> like IUDPUSBApp {
            parameters:
                @display("p=329,54,row,60");
        }
        udp: <udpType> like IUDP if udpType != "" {
            parameters:
                @display("p=329,141");
        }

        usb: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=155,380,row,10;q=txQueue;i=block/ifcard");
        }

    connections allowunconnected:

        udpApp.udpOut --> udp.appIn++;
        udpApp.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn if udpType != "";
        udp.ipIn <-- networkLayer.udpOut if udpType != "";

        if (udpType != "") {
            usbg <--> usb.phys;
            usb.upperLayerOut --> udpApp.USBLayerIn;
            usb.upperLayerIn <-- udpApp.USBLayerOut;
        }
}


