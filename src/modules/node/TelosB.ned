package org.mixim.modules.node;

import inet.base.NotificationBoard;
import inet.linklayer.IWiredNic;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;
import org.mixim.base.modules.IBaseSensorApplLayer;


//
// This NED module defines a host using an IEEE 802.15.4 transceiver at 2.4GHz
// for wireless communications, that can be used to simulate wireless sensor
// networks. 
//
// For instance, you can use it to compare your own MAC protocol to
// the 802.15.4 non beacon enabled MAC protocol, or to evaluate IEEE 802.15.4
// wireless sensor networks. It is also a good starting point to define your
// own Host file.
//
// This model was independently validated on a wireless sensor network testbed.
// For more information, see 
// Accurate Timeliness Simulations for Real-Time Wireless Sensor Networks,
// J. Rousselot, J.-D. Decotignie, M. Aoun, P. van der Stok, R. Serna Oliver,
// G. Fohler. In Proceedings of the 2009 Third UKSim European Symposium on Computer 
// Modeling and Simulation. <a href="http://dx.doi.org/10.1109/EMS.2009.34">http://dx.doi.org/10.1109/EMS.2009.34</a>.
//
// @author J�r�me Rousselot
//
module TelosB extends Host802154_2400MHz
{
    parameters:
        applicationType = "";
        presentationType = "";
        sessionType = "";

        string applSensorType = "WhiteboxApplLayer";
        string sensorType = default("TemperatureSensorLayer");
        string environmentType = "EnvironmentLayer";

        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");
    gates:
        inout usbg;
    submodules:


        notificationBoard: NotificationBoard {
            parameters:
                @display("p=140,302;is=s");
        }

        applSensor: <applSensorType> like IBaseSensorApplLayer if applSensorType != "" {
            parameters:
                @display("p=70,170;i=app;is=l");
            gates:

        }
        sensor: <sensorType> like IBaseNetwLayer if sensorType != "" {
            parameters:
                @display("p=70,100;i=device/antennatower");
        }
        environment: <environmentType> like IBaseApplLayer if environmentType != "" {
            parameters:
                @display("p=70,30;i=misc/sun");
        }

        usb: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=140,380,row,60;q=txQueue");
        }

    connections allowunconnected: // some sensors will not use the usb
        //# finaly we connect the highest existing layer of transport, session, or presentation to
        //# the application layer if specified
        if (applSensorType != "" && presentationType == "" && sessionType == "" && transportType == "") {
            netwl.upperLayerOut --> applSensor.lowerLayerIn;
            netwl.upperLayerIn <-- applSensor.lowerLayerOut;
            netwl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> applSensor.lowerControlIn;
            netwl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- applSensor.lowerControlOut;
        }
        if (applSensorType != "" && presentationType == "" && sessionType == "" && transportType != "") {
            tranl.upperLayerOut --> applSensor.lowerLayerIn;
            tranl.upperLayerIn <-- applSensor.lowerLayerOut;
            tranl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> applSensor.lowerControlIn;
            tranl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- applSensor.lowerControlOut;
        }
//        if (applSensorType != "" && presentationType == "" && sessionType != "") {
//            sessl.upperLayerOut --> applSensor.lowerLayerIn;
//            sessl.upperLayerIn <-- applSensor.lowerLayerOut;
//            sessl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> applSensor.lowerControlIn;
//            sessl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- applSensor.lowerControlOut;
//        }
//        if (applSensorType != "" && presentationType != "") {
//            presl.upperLayerOut --> applSensor.lowerLayerIn;
//            presl.upperLayerIn <-- applSensor.lowerLayerOut;
//            presl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> applSensor.lowerControlIn;
//            presl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- applSensor.lowerControlOut;
//        }

        if (applSensorType != "" && sensorType != "") {
            applSensor.upperLayerOut --> sensor.lowerLayerIn;
            applSensor.upperLayerIn <-- sensor.lowerLayerOut;
            applSensor.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> sensor.lowerControlIn;
            applSensor.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- sensor.lowerControlOut;
        }

        if (environmentType != "" && sensorType != "") {
            sensor.upperLayerOut --> environment.lowerLayerIn;
            sensor.upperLayerIn <-- environment.lowerLayerOut;
            sensor.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> environment.lowerControlIn;
            sensor.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- environment.lowerControlOut;
        }


        if (applSensorType != "") {
            usbg <--> usb.phys;
            usb.upperLayerOut --> applSensor.USBLayerIn;
            usb.upperLayerIn <-- applSensor.USBLayerOut;
        }

//        output deviceLayerOut; // usb interface
//        output deviceControlOut; // usb interface
//        input deviceLayerIn; // usb interface
//        input deviceControlIn

}

