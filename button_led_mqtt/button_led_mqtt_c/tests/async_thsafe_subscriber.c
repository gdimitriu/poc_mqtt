/*
 * async_thsafe_subscriber.c
 *  Created on: Jun 21, 2022
 *      Author: Gabriel Dimitriu
 */

#include <stdio.h>
#include <stdlib.h>
#include <MQTTAsync.h>

#define BROKER "tcp://localhost:1883"
#define CLIENTID "Subscriber_c"
#define TOPIC "sensors/#"
#define QOS 1
#define TIMEOUT 10000L

volatile MQTTAsync_token deliveredToken;

void delivered(void *contex, MQTTAsync_token dt) {
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredToken = dt;
}

int messageArrived(void *context, char *topicName, int topicLen,
		MQTTAsync_message *message) {
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
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

void connLost(void *context, char *cause) {
	printf("\nConnection lost\n with cause: %s\n",cause);
}

void onConnect(void* context, MQTTAsync_successData* response) {
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions responseOps = MQTTAsync_responseOptions_initializer;
	printf("Subscribing to topic %s for client %s using Qos=%d\n",TOPIC,CLIENTID,QOS);
	printf("Press Q<Enter> to quit\n\n");
	responseOps.context = client;
	MQTTAsync_subscribe(client, TOPIC, QOS,&responseOps);
}

int main(int argc,char **argv) {
	MQTTAsync client;
	MQTTAsync_connectOptions connOps = MQTTAsync_connectOptions_initializer;
	MQTTAsync_disconnectOptions disconnectOps = MQTTAsync_disconnectOptions_initializer;
	int rc;
	int ch;

	if ((rc = MQTTAsync_create(&client,BROKER,CLIENTID,MQTTCLIENT_PERSISTENCE_DEFAULT, NULL)) != MQTTASYNC_SUCCESS) {
		printf("Failed to create return code %d\n",rc);
		exit(EXIT_FAILURE);
	}
	connOps.keepAliveInterval = 20;
	connOps.cleansession = 1;
	connOps.username = "mqtt";
	connOps.password = "mqtt";
	connOps.context = client;
	connOps.onSuccess = onConnect;
	MQTTAsync_setCallbacks(client, client, connLost, messageArrived, delivered);
	if ((rc = MQTTAsync_connect(client,&connOps)) != MQTTASYNC_SUCCESS) {
		printf("Failed to connect return code %d\n",rc);
		exit(EXIT_FAILURE);
	}

	do {
		ch = getchar();
	} while(ch !='Q' && ch != 'q');
	disconnectOps.timeout = TIMEOUT;
	MQTTAsync_disconnect(client, &disconnectOps);
	MQTTAsync_destroy(&client);
	return rc;
}

