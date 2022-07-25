
#include "veins/modules/application/traci/VehicleApp.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::VehicleApp);

void VehicleApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();

    }
}

void VehicleApp::onWSM(BaseFrame1609_4* frame)
{
    if(sentMessage){
        return;
    }
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        sendDown(wsm->dup());

    }
}



void VehicleApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 5s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 5 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());
            sendDown(wsm);

        }
    }
    else {
        lastDroveAt = simTime();
    }
}
