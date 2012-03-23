#ifndef EndDevice_h
#define EndDevice_h

#include "../Radio/Radio.h"

// Status codes
#define STATUS_ERROR 1
#define STATUS_SLEEPING 2

// Sleep related pins
#define SLEEP_RQ_PIN 10
#define SLEEP_STATUS_PIN 11

// Tick return values
#define TICK_OK 0
#define TICK_ASSOC_FAIL 1
// TODO: Probably merge the ways to fail joining
#define TICK_JOIN_BAD_MSG 2
#define TICK_JOIN_TIMEOUT 3
#define TICK_JOIN_NOT_DELIVERED 9
#define TICK_JOIN_OK 8
// TODO: Probably merge the ways to fail the update
#define TICK_UPDATE_NO_DELIVERY 4
#define TICK_UPDATE_TIMEOUT 5
#define TICK_NEW_MSG 6
#define TICK_UNKNOWN_ERROR 7
#define TICK_SLEEPING 10

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
		uint8_t tick();

		// Returns a pointer to the data buffer, only use this immediately after
		// (i.e. before calling tick again) tick returns TICK_NEW_MSG
		uint8_t *getData();
	protected:
	private:
		// Storage area for messages
		uint8_t data[216];

		// Pointer to the end of the current data
		uint8_t * dataEnd;

		// The xbee instance to communicate with the radio
		XBee xbee;

		// True if we should request a message update
		bool updateFlag;

		// True if the radio should wake up
		bool wakeupFlag;

		// Internal timer/Timeout handling
		bool hasTimedOut();
		void disableTimeout();
		void setTimeout(unsigned long msecs);

		bool timeOutFlag;
		unsigned long timeOut;

		// Number of times the curreent operation has timed out
		uint8_t timesTimeout;

		// State handling
		EndDeviceState State;
		uint8_t init();
		uint8_t start();
		uint8_t formingNetwork();
		uint8_t joiningSend();
		uint8_t joiningStatus();
		uint8_t joiningWait();
		uint8_t joiningWaitResponse();
		uint8_t idle();
		uint8_t error();
		uint8_t requestSend();
		uint8_t requestWait();
		uint8_t requestStatus();
		uint8_t sleepTell();
		uint8_t sleepWait();
		uint8_t sleeping();
};

#endif
