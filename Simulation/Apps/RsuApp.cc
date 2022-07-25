

#include "veins/modules/application/traci/RsuApp.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::RsuApp);


void RsuApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
        if (stage == 0) {
            sentMessage = false;
            counter = 0;
        }
}
void RsuApp::onWSM(BaseFrame1609_4* frame)
{
    if(!sentMessage)
    {

        TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
        sentMessage = true;
        scheduleAt(simTime() + 2 , wsm->dup());
    }
}

void RsuApp::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        if (counter < 3) {
            counter++;
            sendDown(wsm->dup());
            scheduleAt(simTime() + 10, wsm);

        }
        else {
            counter = 0;
            delete (wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

