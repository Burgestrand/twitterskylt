#ifndef EndDevice_h
#define EndDevice_h

/* Module for the end device of the radio link.
 * Usage example:
 *   #include <EndDevice.h>
 *   EndDevice ed(sleep_rq_pin, sleep_status_pin);
 *  in setup:
 *   ed.begin(serial);
 *  in loop:
 *   switch (ed.tick()) {
 *     case TICK_...
 *   }
 *
 *   1 - Vcc
 *   2 - TX
 *   3 - RX
 *   9 - 4
 *   10 - GND
 *   13 - 5
 */

#include <Arduino.h>
#include <XBee.h>

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
#define TICK_NETWORK_ERROR 11

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
                    , EndDeviceSleepWait
                    , EndDeviceSleeping
                    , EndDeviceWaking
                    , EndDeviceResetStart
                    , EndDeviceResetWait
                    };

/*class EndDevice : public Radio { // Radio no longer a super class, no overlap with it. */
class EndDevice {
	public:
		// Sleep rq pin and sleep status pin should be connected to respective pins
		// on the XBee.
		EndDevice(uint8_t sleep_rq_pin, uint8_t sleep_status_pin);

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
		// Pins for sleep request/status
		uint8_t sleep_rq_pin;
		uint8_t sleep_status_pin;

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

		// True if the radio should reset after waking up
		bool resetFlag;

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
		uint8_t sleepWait();
		uint8_t sleeping();
		uint8_t waking();
		uint8_t resetStart();
		uint8_t resetWait();
};

#endif
