
#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

namespace veins {



class VEINS_API VehicleApp : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    simtime_t lastDroveAt;
    bool sentMessage;


protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void handlePositionUpdate(cObject* obj) override;
};

} // namespace veins
