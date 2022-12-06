//coded by ANIK ANIMESH AKASH

#include "veins_inet/VeinsInetSampleApplication.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "veins_inet/VeinsInetSampleMessage_m.h"

using namespace inet;

Define_Module(VeinsInetSampleApplication);

VeinsInetSampleApplication::VeinsInetSampleApplication()
{
}

bool VeinsInetSampleApplication::startApplication()
{
    // host[0] should stop at t=20s
    if (getParentModule()->getIndex() == 0) {
        auto callback = [this]() {
            getParentModule()->getDisplayString().setTagArg("i", 1, "red");

            traciVehicle->setSpeed(0);

            auto payload = makeShared<VeinsInetSampleMessage>();
            payload->setChunkLength(B(100));
            payload->setRoadId(traciVehicle->getRoadId().c_str());
            timestampPayload(payload);

            auto packet = createPacket("accident");
            packet->insertAtBack(payload);
            sendPacket(std::move(packet));

            // host should continue after 30s
            auto callback = [this]() {
                traciVehicle->setSpeed(-1);
            };
            timerManager.create(veins::TimerSpecification(callback).oneshotIn(SimTime(30, SIMTIME_S)));
        };
        timerManager.create(veins::TimerSpecification(callback).oneshotAt(SimTime(20, SIMTIME_S)));
    }

    return true;
}

bool VeinsInetSampleApplication::stopApplication()
{
    return true;
}

VeinsInetSampleApplication::~VeinsInetSampleApplication()
{
}

void VeinsInetSampleApplication::processPacket(std::shared_ptr<inet::Packet> pk)
{
    auto payload = pk->peekAtFront<VeinsInetSampleMessage>();

    EV_INFO << "Received packet: " << payload << endl;

    getParentModule()->getDisplayString().setTagArg("i", 1, "green");

    traciVehicle->changeRoute(payload->getRoadId(), 999.9);

    if (haveForwarded) return;

    auto packet = createPacket("relay");
    packet->insertAtBack(payload);
    sendPacket(std::move(packet));

    haveForwarded = true;
}
