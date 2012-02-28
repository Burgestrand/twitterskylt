#ifndef EndDevice_h
#define EndDevice_h

#include "Radio.h"

/*
o
States:
Start
Forming network
Joining send
Joining wait
Idle
Error
RequesSend
RequesWait
o
*/

enum EndDeviceState { Start
                    , FormingNetwork
                    , JoiningSend
                    , JoiningWait
                    , Idle
                    , Error
                    , RequestSend
                    , RequestWait
                    };

class EndDevice : public Radio {
	public:
		EndDevice();
		void joinNetwork();
		void getNewestMessage();
		void tick();
		void begin();
	protected:
	private:
		uint8_t * data;
		bool timeOutFlag;
		long timeOut;
		Xbee xbee;

		// State handling
		EndDeviceState State;
		void start();
		void formingNetwork();
		void joiningSend();
		void joiningStatus();
		void joiningWait();
		void idle();
		void error();
		void requestSend();
		void requestWait();
};

#endif