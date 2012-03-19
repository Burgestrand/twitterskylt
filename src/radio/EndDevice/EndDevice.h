#ifndef EndDevice_h
#define EndDevice_h

#include "../Radio/Radio.h"

// Status codes
#define STATUS_ERROR 1
#define STATUS_SLEEPING 2

// Sleep related pins
#define SLEEP_RQ_PIN 10
#define SLEEP_STATUS_PIN 11

// If DEBUG_MSG_FUN is set then DEBUG_MSG will call it with its argument
// DEBUG_MSG_FUN should be a function returning void, taking a char pointer
// as its only argument
#define DEBUG_MSG_FUN debug
#ifdef DEBUG_MSG_FUN
extern "C" { void DEBUG_MSG_FUN(char *); }
#define DEBUG_MSG(MSG) (DEBUG_MSG_FUN(MSG))
#else
#define DEBUG_MSG(MSG) (1)
#endif

// The states the internal state machine can be in
enum EndDeviceState { EndDeviceInit
                    , EndDeviceStart
                    , EndDeviceFormingNetwork
                    , EndDeviceJoiningSend
                    , EndDeviceJoiningWait
                    , EndDeviceJoiningWaitResponse
                    , EndDeviceIdle
                    , EndDeviceError
                    , EndDeviceRequestSend
                    , EndDeviceRequestStatus
                    , EndDeviceRequestWait
                    , EndDeviceSleepTell
                    , EndDeviceSleepWait
                    , EndDeviceSleeping
                    };

class EndDevice : public Radio {
	public:
		// Default constructor, does nothing special
		EndDevice();

		// Set the callback to use when new message has arrived
		void setMsg(void (*msg_callback)(char *));

		// Set the callback to use when the status of the 
		// network changes (associated, failed to deliver message...)
		void setStatus(void (*status_callback)(uint8_t));

		// Start communicating with the xbee
		void begin(long baud);

		// Call to join a network
		void joinNetwork();

		// Call when radio is idle to initiate a request for a new message
		// The new message will be delivered through the message callback
		void getNewestMessage();

		// Wake the radio up
		void wakeup();

		// Call often.
		void tick();
	protected:
	private:

		// Storage area for messages
		uint8_t data[216];

		// Pointer to the end of the current data
		uint8_t * dataEnd;

		// The xbee instance to communicate with the radio
		XBee xbee;

		// Callbacks
		void (*msg_callback)(char *);
		void (*status_callback)(uint8_t);

		// True if we should request a message update
		bool updateFlag;

		// True if the radio should wake up
		bool wakeupFlag;

		// Internal timer/Timeout handling
		bool timeOutFlag;
		unsigned long timeOut;
		bool hasTimedOut();
		void setTimeout(unsigned long msecs);
		void disableTimeout();
		// Number of times the curreent operation has timed out
		uint8_t timesTimeout;

		// State handling
		EndDeviceState State;
		void init();
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
		void sleepTell();
		void sleepWait();
		void sleeping();
};

#endif
