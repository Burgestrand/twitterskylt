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

enum EndDeviceState { EndDeviceStart
                    , EndDeviceFormingNetwork
                    , EndDeviceJoiningSend
                    , EndDeviceJoiningWait
                    , EndDeviceJoiningWaitResponse
                    , EndDeviceIdle
                    , EndDeviceError
                    , EndDeviceRequestSend
                    , EndDeviceRequestStatus
                    , EndDeviceRequestWait
                    };

class EndDevice : public Radio {
	public:
		EndDevice();
		void joinNetwork();
		void getNewestMessage();
		void tick();
		void begin(long baud);
	protected:
	private:
		uint8_t * data;
		bool timeOutFlag;
		long timeOut;
		bool hasTimedOut();
		XBee xbee;

		// State handling
		EndDeviceState State;
		void start();
		void formingNetwork();
		void joiningSend();
		void joiningStatus();
		void joiningWait();
		void joiningWaitResponse();
		void idle();
		void error();
		void requestSend();
		void requestWait();
		void requestStatus();
};

#endif
