/**
 *  Uname 2015/2/11
 *  Aersion 0.1
*/

#include "debug.h"
#include "config.h"
#include "client.h"
#include "net_utils.h"
#include "log.h"

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
		LOG_D("connecting to %s:%d...\n", pstMqtt->server, pstMqtt->port);
		break;
        
	case MQTT_STATE_CONNECTED:
		LOG_I("connected.\n");
		break;
        
	case MQTT_STATE_DISCONNECTED:
		LOG_W("disconnected.\n");
		break;
        
	default:
		LOG_W("it's in badstate.\n");
	}
}

static void onPublish(Mqtt *pstMqtt, void *data, int msgId)
{
	(void)pstMqtt;
	(void)msgId;
	MqttMsg *msg = (MqttMsg *)data;
	LOG_D("publish to %s: %s\n", msg->topic, msg->payload);
}

static void onSubscribe(Mqtt *pstMqtt, void *data, int msgId)
{
    (void)pstMqtt;
	char *topic = (char *)data;
	LOG_D("subscribe to %s: msgId=%d\n", topic, msgId);
}

static void onSuback(Mqtt *pstMqtt, void *data, int msgId)
{
	(void)pstMqtt;
	(void)data;
	LOG_D("received suback: msgId=%d\n", msgId);
}

static void onMessage(Mqtt *pstMqtt, MqttMsg *message)
{
    LOG_I("received message: topic=%s, payload=%s\n", message->topic, message->payload);
    easyMqttPublish(pstMqtt, "apache99", "HiApache!!", 0);
}

static void onDisconnect(Mqtt *pstMqtt, void *data, int id)
{
    LOG_W("disconnected\n");
}

static void setMqttCallbacks(Mqtt *pstMqtt)
{
	int i = 0, type;
    
	MqttCallback callbacks[15] = {
		NULL,
		onConnect,
		NULL, //onConnack,
		onPublish,
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
    LOG_SET_LEVEL(LOG_DEBUG);
    
    srand(time(NULL) ^ getpid());
    
    initClient(&client);
    
    setMqttCallbacks(client.mqtt);
    
    if(mqttConnect(client.mqtt) != 0)
    {
        LOG_E("connect failed");
        exit(1);
    }
    
    LOG_I("connect %s:%d success\n", HOST, PORT);
    
    mqttRun(client.mqtt);
        
    close(client.mqtt->fd);
    
    return 0;
}

