#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "mqtt.h"

typedef struct tagClient {
	Mqtt *mqtt;

} Client;

int initClient(Client *pstClient);
int initMqtt(Mqtt *pstMqtt);

#endif
