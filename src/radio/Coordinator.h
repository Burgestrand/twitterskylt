#ifndef Coordinator_h
#define Coordinator_h

// Time (in seconds) during which new end devices are allowed to join the network
#define PERMIT_JOIN_TIME 5
// Timeout time (in milliseconds) to wait for command and delivery responses
#define TIMEOUT_TIME 5000

#include "Radio.h"

// Internal states for Coordinator
enum State {Start, Init, NetworkFormation, PermitJoining, AwaitJoin, Idle, ModemStatus, Send, Error, ATResponse};

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		// Default constructor
		Coordinator();
		// Begin coordinator operation
		void begin();
		// Pair-up of coordinator and end device
		void pairUp();
		// Set data to be sent upon request from end device
		void setData(uint8_t *data);
	private:
		State state;
		uint8_t *data;
		uint8_t *dataBuffer;
		bool timeOutFlag;
		long timeOut;
			
		// Initialization
		void init();
		// Step State Machine
		void tick();
		// 
		void formNetwork();
		// Permits joining the network for a given period of time (in seconds)
		void permitJoining(uint8_t seconds);
		// 
		void awaitJoin(); 
		//
		void idle();
		//
		void error();
		//
		void sendData();
		//
		void modemStatusAction();
};

#endif
