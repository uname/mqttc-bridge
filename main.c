#include "log.h"
#include "client.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define VERSION_STR "1.0beta"

typedef struct tagOptArgs {
	int baud;
	char *dev;
	char *dev_topic;
	char *remote_topic;
} OptArgs;


static OptArgs optargs = {DEFAULT_BAUD, NULL, NULL, NULL};
static char *optstr = "b:d:t:vh";
static struct option longopts[] = {
	{"baud",      required_argument, NULL, 'b'},
	{"device",    required_argument, NULL, 'd'},
	{"dev-topic", required_argument, NULL, 't'},
	{"version",   no_argument,       NULL, 'v'},
	{"help",      no_argument,       NULL, 'h'},
	{NULL,        no_argument,       NULL, 0}
};

void showVersion()
{
	printf("mqttc-bridge version %s\n", VERSION_STR);
}

void showHelp()
{
	 printf("Usage: mqtt-bridge [OPTIONS] \n");
}

int getOptions(int argc, char *argv[])
{
	int opt;

	while((opt = getopt_long(argc, argv, 
		optstr, longopts, NULL)) != -1) {

		switch(opt) {
		case 'b':
			optargs.baud = atoi(optarg);
			break;

		case 'd':
			optargs.dev = strdup(optarg);
			break;

		case 't':
			optargs.dev_topic = strdup(optarg);
			break;

		case 'v':
			showVersion();
			exit(0);

		case 'h':
			showHelp();
			exit(0);

		default:
			break;
		}
	}

	if(optargs.dev == NULL) {
		fprintf(stderr, "serial device is required!\n");
		return -1;
	}

	if(optargs.dev_topic == NULL) {
		fprintf(stderr, "device topic to be discribed is required!\n");
		return -1;
	}

	return 0;

}


int main(int argc, char *argv[])
{
	if(getOptions(argc, argv) != 0)	{
		showHelp();
		exit(0);
	}

	return clientRun();
}
