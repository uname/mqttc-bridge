#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "mqtt.h"
#include "serial.h"

typedef struct tagClient {
    Serial *serial;
	Mqtt *mqtt;

} Client;

int clientRun();
int initClient(Client *pstClient);
int initMqtt(Mqtt *pstMqtt);
int initSerial(Serial *pstSerial);

#endif
