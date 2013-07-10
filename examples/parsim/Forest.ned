//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.mixim.examples.parsim;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.Eth100M;
import ned.DatarateChannel;
import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseNetwork;
import org.mixim.modules.node.EG;
import org.mixim.modules.node.TelosB;


network Forest extends BaseNetwork
{
    parameters:
        int numNodesNetworkOne; // total number of hosts in the network One
        int numNodesNetworkTwo; // total number of hosts in the network Two

    submodules:

        networkOne[numNodesNetworkOne]: TelosB {
            parameters:
                numHosts = numNodesNetworkOne;
                @display("p=161,50;i=device/wifilaptop;b=5,5");
        }

        enhancedGatewayOne: EG {
            @display("p=50,50");
        }

        networkTwo[numNodesNetworkTwo]: TelosB {
            parameters:
                numHosts = numNodesNetworkTwo;
                @display("p=170,134;i=device/wifilaptop;b=5,5");
        }

        enhancedGatewayTwo: EG {
            @display("p=50,134");
        }

        SecondChannel: ConnectionManager {
            parameters:
                @display("p=121,10;i=abstract/multicast;is=s");
        }


        configurator: IPv4NetworkConfigurator {
            config = xml("<config>" +
            				"<interface hosts='*' address='10.0.0.x' netmask='255.255.255.x'/> " + 
            				"<multicast-group hosts='enhancedGateway*' address='224.0.0.1' /> " +
            			"</config>");
            @display("p=65,10");
        }

    connections:

        enhancedGatewayOne.usbg <--> USB <--> networkOne[0].usbg;
        enhancedGatewayTwo.usbg <--> USB <--> networkTwo[0].usbg;

        enhancedGatewayOne.ethg++ <--> Eth100M <--> enhancedGatewayTwo.ethg++;

}

channel USB extends DatarateChannel
{
    parameters:
        datarate = 480Mbps;
        delay = 0.000026ms;
        per = 0;
        ber = 0;
}
