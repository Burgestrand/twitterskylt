#ifndef Coordinator_h
#define Coordinator_h

// Time (in seconds) during which new end devices are allowed to join the network
#define PERMIT_JOIN_TIME 5
// Timeout time (in milliseconds) to wait for command and delivery responses
#define TIMEOUT_TIME 5000

// Status codes
#define COORDINATOR_OK 0
#define COORDINATOR_ERROR_NETWORK 1
#define COORDINATOR_ERROR_AT_COMMAND 2
#define COORDINATOR_ERROR_UNEXPECTED 3

#include "Radio.h"

// Internal states for Coordinator
enum State {	NoStart, 
		Start, 
		Init, 
		NetworkFormationSend, 
		NetworkFormationReceive, 
		PermitJoiningSend, 
		PermitJoiningReceive, 
		AwaitJoin, 
		JoinResponse, 
		JoinResponseDelivery, 
		Idle, 
		SendData, 
		SendDataDelivery, 
		ModemStatusAction, 
		Error
	};

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		/* Default constructor */
		Coordinator();
		/* Begin coordinator operation */
 		void begin();
		/* Pair-up of coordinator and end device */
		void pairUp();
		/* Set data to be sent to end device upon request */
		void setData(uint8_t *data);
		/* Step State Machine */
		void tick();
		/* Return current status,
		 *
		 * 0 - Everything OK!
		 * 1 - Network/Connection Error
		 * 2 - AT Command Error
		 * 3 - Unexpected Error
		 */
		uint8_t status();
	private:
		State state;
		uint8_t *data;
		uint8_t *dataBuffer;
		uint8_t status;
		bool timeOutFlag;
		long timeOut;
			
		// Initialization
		void init();
		// Start timeout timer
		void startTimeOut();
		// Check timer to see if we've timed out
		void checkTimeOut();
		// Send AT command cmd, then move on to nextState
		void sendAtCommand(uint8_t *cmd, State nextState);
		// Await response from a sent AT command, then move to nextState
		void awaitAtResponse(State nextState) ;
		// Wait for an end device to join network
		void awaitJoin(); 
		// Send confirmation to end device attempting to join
		void joinResponse();
		// Receive package delivery report
		void dataDeliveryStatus();
		// Idle, listen for incoming packages
		void idle();
		// Send data to end device
		void sendData();
		// Act on modem status response from local XBee
		void modemStatusAction();
		// Signal error code and wait for reset
		void error();
};

#endif
