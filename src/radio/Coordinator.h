#ifndef Coordinator_h
#define Coordinator_h

#define PERMIT_JOIN_TIME 5
#define TIMEOUT_TIME 5000

#include "Radio.h"

enum State {Start, Init, NetworkFormation, PermitJoining, AwaitJoin, Idle, ModemStatus, Send, Error, ATResponse};

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		// Default Constructor
		Coordinator();
		// Begin
		void begin();
		// External Pair-Up
		void pairUp();
		// Set
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


		// Check for data request from end device
		bool gotDataRequest();

};

#endif
