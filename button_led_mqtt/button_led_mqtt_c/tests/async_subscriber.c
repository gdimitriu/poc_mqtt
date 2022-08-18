/*
 * async_subscriber.c
 *  Created on: Jun 21, 2022
 *      Author: Gabriel Dimitriu
 */

#include <stdio.h>
#include <stdlib.h>
#include <MQTTClient.h>

#define BROKER "tcp://localhost:1883"
#define CLIENTID "Subscriber_c"
#define TOPIC "sensors/#"
#define QOS 1
#define TIMEOUT 10000L

volatile MQTTClient_deliveryToken deliveredToken;

void delivered(void *contex, MQTTClient_deliveryToken dt) {
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredToken = dt;
}

int messageArrived(void *context, char *topicName, int topicLen,
		MQTTClient_message *message) {
	int i;
	char *payloadPtr;
	printf("Message arrived %s on topic %s\n", (char*) message->payload,
			topicName);
	payloadPtr = message->payload;
	printf("Real message: ");
	for (int i = 0; i < message->payloadlen; i++) {
		putchar(*payloadPtr++);
	}
	putchar('\n');
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connLost(void *context, char *cause) {
	printf("\nConnection lost\n with cause: %s\n",cause);
}

int main(int argc,char **argv) {
	MQTTClient client;
	MQTTClient_connectOptions connOps = MQTTClient_connectOptions_initializer;
	int rc;
	int ch;

	MQTTClient_create(&client,BROKER,CLIENTID,MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
	connOps.keepAliveInterval = 20;
	connOps.cleansession = 1;
	connOps.username = "mqtt";
	connOps.password = "mqtt";
	MQTTClient_setCallbacks(client, NULL, connLost, messageArrived, delivered);
	if ((rc = MQTTClient_connect(client,&connOps)) != MQTTCLIENT_SUCCESS) {
		printf("Failed to connect return code %d\n",rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribing to topic %s for client %s using Qos=%d\n",TOPIC,CLIENTID,QOS);
	printf("Press Q<Enter> to quit\n\n");
	MQTTClient_subscribe(client, TOPIC, QOS);
	do {
		ch = getchar();
	} while(ch !='Q' && ch != 'q');
	MQTTClient_disconnect(client, TIMEOUT);
	MQTTClient_destroy(&client);
	return rc;
}

