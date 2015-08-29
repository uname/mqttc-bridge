/**
 *  Uname 2015/2/11
 *  Aersion 0.1
*/

#include "debug.h"
#include "config.h"
#include "client.h"
#include "net_utils.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Client client;

int initMqtt(Mqtt *pstMqtt)
{
    char clientId[23] = {0};
    if(pstMqtt == NULL)
    {
        return -1;
    }
	pstMqtt->state = 0;
    sprintf(clientId, "mqttc%d", rand());
	pstMqtt->clientId = strdup(clientId);
    
    pstMqtt->port = PORT;
    mqttSetServer(pstMqtt, HOST);
    pstMqtt->msgId = 1;
    
    return 0;
}

int initClient(Client *pstClient)
{
    if(pstClient == NULL)
    {
        return -1;
    }
    
    memset(pstClient, 0, sizeof(Client));
    pstClient->mqtt = mqttNew();
    initMqtt(pstClient->mqtt);
    
    return 0;
}

static void onConnect(Mqtt *pstMqtt, void *data, int state)
{
	(void)data;
    
	switch(state)
    {
	case MQTT_STATE_CONNECTING:
		printf("mqttc is connecting to %s:%d...\n", pstMqtt->server, pstMqtt->port);
		break;
        
	case MQTT_STATE_CONNECTED:
		printf("mqttc is connected.\n");
		break;
        
	case MQTT_STATE_DISCONNECTED:
		printf("mqttc is disconnected.\n");
		break;
        
	default:
		printf("mqttc is in badstate.\n");
	}
}

static void onSubscribe(Mqtt *pstMqtt, void *data, int msgid)
{
    (void)pstMqtt;
	char *topic = (char *)data;
	printf("subscribe to %s: msgid=%d\n", topic, msgid);
}

static void onSuback(Mqtt *pstMqtt, void *data, int msgid)
{
	(void)pstMqtt;
	(void)data;
	printf("received suback: msgid=%d\n", msgid);
}

static void onMessage(Mqtt *pstMqtt, MqttMsg *message)
{
    printf("received message: topic=%s, payload=%s\n", message->topic, message->payload);
}

static void onDisconnect(Mqtt *pstMqtt, void *data, int id)
{
    fprintf(stderr, "Disconnected\n");
}

static void setMqttCallbacks(Mqtt *pstMqtt)
{
	int i = 0, type;
    
	MqttCallback callbacks[15] = {
		NULL,
		onConnect,
		NULL, //onConnack,
		NULL, //onPublish,
		NULL, //onPuback,
		NULL, //onPubrec,
		NULL, //onPubrel,
		NULL, //onPubcomp,
		onSubscribe,
		onSuback,
		NULL, //onUnsubscribe,
		NULL, //onUnsuback,
		NULL, //onPingreq,
		NULL, //onPingresp,
		onDisconnect
	};
	for(i = 0; i < 15; i++) {
		type = (i << 4) & 0xf0;
		mqttSetCallback(pstMqtt, type, callbacks[i]);
	}
    
    pstMqtt->msgCallback = onMessage;
}

int main(int argc, char **argv)
{
    srand(time(NULL) ^ getpid());
    
    initClient(&client);
    
    setMqttCallbacks(client.mqtt);
    
    if(mqttConnect(client.mqtt) != 0)
    {
        fprintf(stderr, "Connect failed\n");
        exit(1);
    }
    
    DEBUG_PRINT("Connect %s:%d success\n", HOST, PORT);
    
    mqttLoop(client.mqtt);
        
    close(client.mqtt->fd);
    
    return 0;
}

