#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#define MQTT_CALLBACK_NUM  16
#define MQTT_PROTO_MAJOR   3
#define MQTT_PROTO_MINOR   1

/*
 * MQTT ConnAck
 */
typedef enum {
	CONNACK_ACCEPT  = 0,
	CONNACK_PROTO_VER, 
	CONNACK_INVALID_ID,
	CONNACK_SERVER,
	CONNACK_CREDENTIALS,
	CONNACK_AUTH
} ConnAck;

/*
 * MQTT State
 */
typedef enum {
	MQTT_STATE_INIT = 0,
	MQTT_STATE_CONNECTING,
	MQTT_STATE_CONNECTED,
	MQTT_STATE_DISCONNECTED
} MqttState;

/*
 * MQTT QOS
 */
#define MQTT_QOS0 0
#define MQTT_QOS1 1
#define MQTT_QOS2 2

typedef struct tagMqtt Mqtt;
typedef void (*MqttCallback)(Mqtt *pstMqtt, void *data, int id);

typedef struct tagMqttMsg {
	uint16_t id;
	uint8_t qos;
	bool retain;
	bool dup;
	const char *topic;
	int payloadlen;
	const char *payload;
} MqttMsg;

typedef void (*MqttMsgCallback)(Mqtt *pstMqtt, MqttMsg *message);

struct tagMqtt {
    uint8_t state;
    int fd;         // socket fd
    const char *clientId;
	char *server;
	int port;
	void *userdata;
    
    unsigned int keepalive;
    int msgId;
    MqttCallback callbacks[MQTT_CALLBACK_NUM];
    MqttMsgCallback msgCallback;
};


Mqtt *mqttNew(void);
void mqttSetServer(Mqtt *pstMqtt, const char *server);
int mqttConnect(Mqtt *pstMqtt);
void mqttRead(Mqtt *pstMqtt);

void mqttSetState(Mqtt *pstMqtt, int state);
void mqttSetCallback(Mqtt *pstMqtt, uint8_t type, MqttCallback callback);

int easyMqttPublish(Mqtt *pstMqtt, char *topic, char *payload, size_t size, int qos);
int mqttPublish(Mqtt *pstMqtt, MqttMsg *msg);
int mqttSubscribe(Mqtt *pstMqtt, const char *topic, unsigned char qos);

MqttMsg *makeMqttMsg(int msgId, int qos, bool retain, bool dup, 
			         char *topic, int payloadlen, char *payload);
void mqttKeepalive(Mqtt *pstMqtt);
#endif

