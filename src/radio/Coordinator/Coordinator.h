#ifndef Coordinator_h
#define Coordinator_h

// PAN ID 129
// Time (in seconds) during which new end devices are allowed to join the network
#define PERMIT_JOIN_TIME 0xFF
// Timeout time (in milliseconds) to wait for command and delivery responses
#define TIMEOUT_TIME 1000
#define JOIN_INTERVAL 65000;

// The maximum number of bytes that fit in a single XBee packet
#define MAX_PACKET_SIZE 72;

#include "../Radio/Radio.h"	

// Internal states for Coordinator
enum CoordinatorState {	
		CoordinatorNoStart, 
		CoordinatorStart, 
		CoordinatorInit, 
		CoordinatorNetworkFormationSend, 
		CoordinatorNetworkFormationReceive, 
		CoordinatorPermitJoiningSend, 
		CoordinatorPermitJoiningReceive, 
		CoordinatorAwaitJoin, 
		CoordinatorJoinResponse, 
		CoordinatorJoinResponseDelivery, 
		CoordinatorIdle, 
		CoordinatorSendData, 
		CoordinatorSendDataDelivery, 
		CoordinatorModemStatusAction, 
		CoordinatorError
	};

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		/* Default constructor */
		Coordinator();
		/* Begin coordinator operation */
 		void begin(HardwareSerial &serialPort);
		/* Pair-up of coordinator and end device */
		void pairUp();
		/* Set data to be sent to end device upon request */
		void setData(uint8_t *data, uint8_t dataSize);
		/* Set callback function for when an error is encountered */
		void setErrorCallback(void (*callbackPt)(void));
		/* Step State Machine */
		void tick();

		// Debug functions
		uint8_t getState();
		String getStateName(uint8_t stateNum);
		bool getAssoc();

	private:
		CoordinatorState state;
		uint8_t **data;
		uint8_t dataSize;
		uint8_t *packetSize;
		uint8_t **dataBuffer;
		uint8_t dataBufferSize;
		uint8_t *packetBufferSize;
		uint8_t currentPacket;
		bool sending;
		bool timeOutFlag;
		long timeOut;
		void (*callbackPt)(void);
			
		bool assoc;

		// Initialization
		void init(); 
		// Start timeout timer
		void startTimeOut(uint16_t timeoutTime = TIMEOUT_TIME); 
		// Check timer to see if we've timed out
		void checkTimeOut();
		// Send AT command cmd, then move on to nextState
		void sendAtCommand(uint8_t *cmd, CoordinatorState nextState);
		// Await response from a sent AT command, then move to nextState
		void awaitAtResponse(CoordinatorState nextState) ;
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
		// Send single packet
		void sendPacket(uint8_t packet);
		// Act on modem status response from local XBee
		void modemStatusAction();
		// Signal error code and wait for reset
		void error();
};

#endif
