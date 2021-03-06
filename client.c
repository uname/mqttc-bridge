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
#include <sys/time.h> 

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

int initSerial(Serial *pstSerial)
{
    pstSerial->dev = strdup(SERIAL_DEV);
    pstSerial->fd = serialInit(pstSerial->dev, pstSerial->baud);
    if(pstSerial->fd < 0)
    {
        LOG_E("fail to init serial\n");
        return -1;
    }
    
    LOG_D("init serial ok\n");
    
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
    pstClient->serial = serialNew();
    initMqtt(pstClient->mqtt);
    initSerial(pstClient->serial);
    
    return 0;
}

static void onConnect(Mqtt *pstMqtt, void *data, int state)
{
	(void)data;
    
	switch(state)
    {
	case MQTT_STATE_CONNECTING:
		LOG_D("connecting to %s:%d ...\n", pstMqtt->server, pstMqtt->port);
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

static void onPingreq(Mqtt *pstMqtt, void *data, int msgId)
{
    (void)pstMqtt;
	(void)data;
	LOG_D("on ping req\n");
}

static void onPingresp(Mqtt *pstMqtt, void *data, int msgId)
{
    (void)pstMqtt;
	(void)data;
	LOG_D("on ping response\n");
}

static void onMessage(Mqtt *pstMqtt, MqttMsg *message)
{
    LOG_I("received message: topic=%s, payload=%s\n", message->topic, message->payload);
    if(client.serial != NULL && client.serial->fd > 0) {
        LOG_D("trans to serial, buff size = %d\n", message->payloadlen);
        write(client.serial->fd, message->payload, message->payloadlen);
    }
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
		onPingreq,
		onPingresp,
		onDisconnect
	};
	for(i = 0; i < 15; i++) {
		type = (i << 4) & 0xf0;
		mqttSetCallback(pstMqtt, type, callbacks[i]);
	}
    
    pstMqtt->msgCallback = onMessage;
}

void clientLoop(Client *pstClient)
{
    fd_set fdset;
    struct timeval tv;
	struct timeval tick;
    time_t last_sec = 0;
    int ret;
    int maxfd = pstClient->mqtt->fd > pstClient->serial->fd ? pstClient->mqtt->fd : pstClient->serial->fd;
    
    if(pstClient == NULL) {
        LOG_E("client is null");
        return;
    }
   
    gettimeofday(&tick, NULL);
	last_sec = tick.tv_sec;

	write(pstClient->serial->fd, START_CODE, strlen(START_CODE));

    while(1) {
        gettimeofday(&tick, NULL);
        if(tick.tv_sec - last_sec > 6) {
            mqttKeepalive(pstClient->mqtt);
            last_sec = tick.tv_sec;
        }

        FD_ZERO(&fdset);
        FD_SET(pstClient->mqtt->fd, &fdset);
        FD_SET(pstClient->serial->fd, &fdset);
        
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        ret = select(maxfd + 1, &fdset, NULL, NULL, &tv);
        if(ret < 0) {
            LOG_E("select error\n");
            break;
        } else if (ret == 0) {
            continue;
        }
        
        if(FD_ISSET(pstClient->mqtt->fd, &fdset)) {
            mqttRead(pstClient->mqtt);
        }
        
        if(FD_ISSET(pstClient->serial->fd, &fdset)) {
            if(serialRead(pstClient->serial) < 1) {
                LOG_E("read serial error\n");
                continue;
            }
            
            // currently, just ignore the message from serial device
            //easyMqttPublish(pstClient->mqtt, "apache001", pstClient->serial->buff, pstClient->serial->bufflen, 0);
        }
    }
}

int clientRun()
{
    LOG_SET_LEVEL(LOG_DEBUG);
    
    srand(time(NULL) ^ getpid());
    
    initClient(&client);
    
    setMqttCallbacks(client.mqtt);
    
    if(mqttConnect(client.mqtt) != 0) {
        LOG_E("connect failed");
        exit(1);
    }
    
    LOG_I("connect %s:%d success\n", HOST, PORT);
    
    clientLoop(&client);
        
    close(client.mqtt->fd);
    close(client.serial->fd);
    
    return 0;
}

