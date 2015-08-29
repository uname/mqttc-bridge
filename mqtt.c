#include "mqtt.h"
#include "net_utils.h"
#include "packet.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MQTT_BUFFER_SIZE   1024
#define KEEPALIVE          300
#define KEEPALIVE_TIMEOUT  ((KEEPALIVE) << 1)

Mqtt *mqttNew()
{
    Mqtt *mqtt;
    
    mqtt = (Mqtt *)malloc(sizeof(Mqtt));
    if(mqtt == NULL)
    {
        fprintf(stderr, "FATAL: No enough memory\n");
        exit(1);
    }
    mqtt->fd = 0;
    mqtt->clientId = NULL;
    mqtt->server = NULL;
    mqtt->port = 0;
    mqtt->userdata = NULL;
    mqtt->keepalive = KEEPALIVE;
    
    return mqtt;
}

void mqttSetServer(Mqtt *pstMqtt, const char *server)
{
	pstMqtt->server = strdup(server);
}

void mqttSetState(Mqtt *pstMqtt, int state)
{
    if(pstMqtt == NULL)
    {
        return;
    }
    pstMqtt->state = state;
}

void mqttSetCallback(Mqtt *pstMqtt, uint8_t type, MqttCallback callback)
{
    if(type < 0)
    {
        return;
    }
	type = (type >> 4) & 0x0F;
	if(type > 16)
    {
        return;
    }
	pstMqtt->callbacks[type] = callback;
}


static void mqttSendConnect(Mqtt *pstMqtt)
{
	int len = 0;
	char *ptr, *buffer = NULL;

	uint8_t header = CONNECT;
	uint8_t flags = 0;

	int remaining_count = 0;
	char remaining_length[4];

	//header
	header = SETQOS(header, MQTT_QOS1);
	
	//flags
	flags = FLAG_CLEANSESS(flags, 0);
	flags = FLAG_WILL(flags, 0);

	//length
	if(pstMqtt->clientId)
    {
		len = 12 + 2 + strlen(pstMqtt->clientId);
	}
    
	remaining_count = _encode_remaining_length(remaining_length, len);

	ptr = buffer = malloc(1 + remaining_count + len);
	
	_write_header(&ptr, header);
	_write_remaining_length(&ptr, remaining_length, remaining_count);
	_write_string_len(&ptr, PROTOCOL_MAGIC, 6);
	_write_char(&ptr, MQTT_PROTO_MAJOR);
	_write_char(&ptr, flags);
	_write_int(&ptr, pstMqtt->keepalive);
	_write_string(&ptr, pstMqtt->clientId);

	write(pstMqtt->fd, buffer, ptr - buffer);

	free(buffer);
}

static void mqttSendSubscribe(Mqtt *pstMqtt, int msgid, const char *topic, uint8_t qos)
{
	int len = 0;
	char *ptr, *buffer;

	int remaining_count;
	char remaining_length[4];

	uint8_t header = SETQOS(SUBSCRIBE, MQTT_QOS1);

	len += 2; //msgid
	len += 2 + strlen(topic) + 1; //topic and qos

	remaining_count = _encode_remaining_length(remaining_length, len);
	ptr = buffer = malloc(1 + remaining_count + len);
	
	_write_header(&ptr, header);
	_write_remaining_length(&ptr, remaining_length, remaining_count);
	_write_int(&ptr, msgid);
	_write_string(&ptr, topic);
	_write_char(&ptr, qos);

	write(pstMqtt->fd, buffer, ptr-buffer);

	free(buffer);
}

static void mqttCallback(Mqtt *pstMqtt, int type, void *data, int id) {
	if(type < 0)
    {
        return;
    }
	type = (type >> 4) & 0x0F;
	if(type > 16)
    {
        return;
    }
	MqttCallback cb = pstMqtt->callbacks[type];
	if(cb)
    {
        cb(pstMqtt, data, id);
    }
}

/**
 * 处理服务器的连接回包
*/
static void mqttHandleConnack(Mqtt *pstMqtt, int rc)
{
	mqttCallback(pstMqtt, CONNACK, NULL, rc);
	if(rc == CONNACK_ACCEPT)
    {
		mqttSetState(pstMqtt, MQTT_STATE_CONNECTED);
		mqttCallback(pstMqtt, CONNECT, NULL, MQTT_STATE_CONNECTED);
        
        mqttSubscribe(pstMqtt, TOPIC, QOS);
	}
}

static void mqttHandlePublish(Mqtt *pstMqtt, MqttMsg *msg)
{
	switch(msg->qos)
    {
    case MQTT_QOS1:
		//mqtt_puback(mqtt, msg->id);
        break;
        
    case MQTT_QOS2:
		//mqtt_pubrec(mqtt, msg->id);
        break;
    }
    
	if(pstMqtt->msgCallback)
    {
        pstMqtt->msgCallback(pstMqtt, msg);
	}
    else
    {
        fprintf(stderr, "No message callback");
    }
    
    if(msg->topic)
    {
        free((void *)msg->topic);
	}
    if(msg->payload)
    {
        free((void *)msg->payload);
	}
    free(msg);
}

static void mqttHandleSuback(Mqtt *pstMqtt, int msgid, int qos)
{
	(void)qos;
	mqttCallback(pstMqtt, SUBACK, NULL, msgid);
}

static MqttMsg * mqtt_msg_new(int msgid, int qos, bool retain, bool dup, 
			                  char *topic, int payloadlen, char *payload)
{
	MqttMsg *msg = malloc(sizeof(MqttMsg));
	msg->id = msgid;
	msg->qos = qos;
	msg->retain = retain;
	msg->dup = dup;
	msg->topic = topic;
	msg->payloadlen = payloadlen;
	msg->payload = payload;
    
	return msg;
}

/**
 * 处理服务器回包
*/
static void mqttHandlePacket(Mqtt *mqtt, uint8_t header, char *buffer, int buflen)
{
	int qos, msgid=0;
	bool retain, dup;
	int topiclen = 0;
	char *topic = NULL;
	char *payload = NULL;
	int payloadlen = buflen;
	MqttMsg *msg = NULL;
	uint8_t type = GETTYPE(header);
    
	switch (type) {
	case CONNACK:
		_read_char(&buffer);
		mqttHandleConnack(mqtt, _read_char(&buffer));
		break;
  
	case PUBLISH:
		qos = GETQOS(header);;
		retain = GETRETAIN(header);
		dup = GETDUP(header);
		topic = _read_string_len(&buffer, &topiclen);
		payloadlen -= (2+topiclen);
		if( qos > 0)
        {
			msgid = _read_int(&buffer);
			payloadlen -= 2;
		}
		payload = malloc(payloadlen+1);
		memcpy(payload, buffer, payloadlen);
		payload[payloadlen] = '\0';
		msg = mqtt_msg_new(msgid, qos, retain, dup, topic, payloadlen, payload);
		mqttHandlePublish(mqtt, msg);
		break;
    /*
        
	case PUBACK:
	case PUBREC:
	case PUBREL:
	case PUBCOMP:
		msgid = _read_int(&buffer);
		_mqtt_handle_puback(mqtt, type, msgid);
		break;
    */
	case SUBACK:
		msgid = _read_int(&buffer);
		qos = _read_char(&buffer);
		mqttHandleSuback(mqtt, msgid, qos);
		break;
    /*  
	case UNSUBACK:
		msgid = _read_int(&buffer);
		_mqtt_handle_unsuback(mqtt, msgid);
		break;
        
	case PINGRESP:
		_mqtt_handle_pingresp(mqtt);
		break;
        
	default:
		_mqtt_set_error(mqtt->errstr, "badheader: %d", type); */
	}
}

static void mqttReaderFeed(Mqtt *pstMqtt, char *buffer, int len)
{
	uint8_t header;
	char *ptr = buffer;
	int remaining_length;
	int remaining_count;
	
	header = _read_header(&ptr);
	
	remaining_length = _decode_remaining_length(&ptr, &remaining_count);
	if((1 + remaining_count + remaining_length) != len )
    {
		fprintf(stderr, "data length error");
		return;
	}
    
	mqttHandlePacket(pstMqtt, header, ptr, remaining_length);
}

static void mqttRead(Mqtt *pstMqtt)
{
    int nread, timeout;
	char buffer[MQTT_BUFFER_SIZE];

    nread = read(pstMqtt->fd, buffer, MQTT_BUFFER_SIZE);
    if (nread < 0)
    {
        if (errno == EAGAIN)
        {
            printf("error1\n");
            return;
        }
        else
        {
            printf("error2\n");
			//mqtt->error = errno;
			//_mqtt_set_error(mqtt->errstr, "socket error: %d.", errno);
        }
    }
    else if (nread == 0)
    {
        //mqtt_disconnect(mqtt);
        //printf("error3\n");
		//timeout = (random() % 300) * 1000;
		//aeCreateTimeEvent(el, timeout, _mqtt_reconnect, mqtt, NULL);
    }
    else
    {
        mqttReaderFeed(pstMqtt, buffer, nread);
    }
}

int mqttConnect(Mqtt *pstMqtt)
{
    struct sockaddr_in sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(pstMqtt->port);
    if(netutil_resolve(&sa, pstMqtt->server) != 0)
    {
        fprintf(stderr, "FATAL: Convert IP error\n");
        exit(1);
    }
    
    pstMqtt->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(pstMqtt->fd == -1)
    {
        fprintf(stderr, "Create client socket error\n");
        return -1;
    }
    
    if(connect(pstMqtt->fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    {
        fprintf(stderr, "Connect error\n");
        return -1;
    }
    
    mqttSendConnect(pstMqtt);
    mqttSetState(pstMqtt, MQTT_STATE_CONNECTING);
    mqttCallback(pstMqtt, CONNECT, NULL, MQTT_STATE_CONNECTING);
    
    return 0;
}

//SUBSCRIBE
int mqttSubscribe(Mqtt *pstMqtt, const char *topic, unsigned char qos)
{
	int msgId = pstMqtt->msgId++;
	mqttSendSubscribe(pstMqtt, msgId, topic, qos);
	mqttCallback(pstMqtt, SUBSCRIBE, (void *)topic, msgId);
    return msgId;
}

void mqttLoop(Mqtt *pstMqtt)
{
    fd_set fdset;
    struct timeval tv;
    int ret;
    int maxFd = pstMqtt->fd;
    
    while(1)
    {
        FD_ZERO(&fdset);
        FD_SET(pstMqtt->fd, &fdset);
        
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        ret = select(maxFd + 1, &fdset, NULL, NULL, &tv);
        if(ret < 0)
        {
            fprintf(stderr, "Select error\n");
            break;
        }
        else if (ret == 0) {
            continue;
        }
        
        if(FD_ISSET(pstMqtt->fd, &fdset))
        {
            mqttRead(pstMqtt);
        }
    }
}
