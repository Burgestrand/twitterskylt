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

// Tick return values
#define TICK_OK 0
#define TICK_ERROR 1
#define TICK_AT_CMD_ERROR 2
#define TICK_TIMEOUT 3
#define TICK_SEND_NO_DELIVERY 4
#define TICK_UNEXPECTED_PACKET 5
#define TICK_MODEM_ERROR 6
#define TICK_UNEXPECTED_MODEM_RESPONSE 7
#define TICK_NETWORK_FORMATION_ERROR 8
#define TICK_UNKNOWN_ERROR 9

#include "../Radio/Radio.h"	

typedef struct struct_packet packet; 

// ZigBee Data Packet
struct struct_packet {
	uint8_t data[72];
	uint8_t length;
};

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
		CoordinatorError,
		CoordinatorTimeOut
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
		/* Step State Machine */
		uint8_t tick();

		// Debug functions
		uint8_t getState();
		String getStateName(uint8_t stateNum);
		bool getAssoc();

	private:
		CoordinatorState state;
		packet dataPackets[3];
		packet bufferPackets[3];
		uint8_t currentPacket;
		bool sending;
		bool timeOutFlag;
		unsigned long timeOut;
		bool assoc;

		// Initialization
		uint8_t init(); 
		// Start timeout timer
		void startTimeOut(uint16_t timeoutTime = 2000); 
		// Check timer to see if we've timed out
		void checkTimeOut();
		// Send AT command cmd, then move on to nextState
		uint8_t sendAtCommand(uint8_t *cmd, CoordinatorState nextState);
		// Await response from a sent AT command, then move to nextState
		uint8_t awaitAtResponse(CoordinatorState nextState) ;
		// Wait for an end device to join network
		uint8_t awaitJoin(); 
		// Send confirmation to end device attempting to join
		uint8_t joinResponse();
		// Receive package delivery report
		uint8_t dataDeliveryStatus();
		// Idle, listen for incoming packages
		uint8_t idle();
		// Send data to end device
		uint8_t sendData();
		// Send single packet
		uint8_t sendPacket(packet dataPacket);
		// Act on modem status response from local XBee
		uint8_t modemStatusAction();
		// General error
		uint8_t error();
		// Timeout error
		uint8_t timeout();
};

#endif
