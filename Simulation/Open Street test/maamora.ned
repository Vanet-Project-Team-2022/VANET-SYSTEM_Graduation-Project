// -----------------------------------------------------------------
// File: maamora.ned
//
// NOTES: it could be necessary to do the following:
// 1. To include this folder (or an upper one) in the list of
//    .ned folders from the project properties:
//        OMNeT++ > NED Source Folder.
// 2. To specify a 'package' line if there exist same package.ned
//    file in the working directory (or an upper one).
// -----------------------------------------------------------------

package maamora.simulations.maamora;

import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Scenario;

network maamora extends Scenario
{
    parameters:
        @display("bgb=$playgroundSizeX,$playgroundSizeY");

        bool withObstacles = default(true);

    submodules:
        rsu[1]: RSU {
            @display("p=150,140;b=10,10,oval");
        }

    connections allowunconnected:
}

