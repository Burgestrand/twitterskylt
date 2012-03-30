#include "Common/Common.h"
#include "Coordinator/Coordinator.h"
#include "Ethernet/Ethernet.h"
#include "HTTP.h"

Coordinator coordinator;

void setup(void)
{
	coordinator.begin(Serial1);
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	Ethernet.begin(mac);
}

void loop(void)
{
	Ethernet.renew();
    coordinator.tick();
    HTTP.tick();
}
