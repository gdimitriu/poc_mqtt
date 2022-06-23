/*
 * button_led_mqtt.cpp
 *  Created on: Jun 21, 2022
 *      Author: Gabriel Dimitriu
 */

#include <stdio.h>
#include <stdlib.h>
#include <MQTTClient.h>
#include <string>
#include <map>

using namespace std;

#define BROKER "tcp://localhost:1883"
#define CLIENTID "Subscriber_c"
#define QOS 1
#define TIMEOUT 10000L

const string topicSensors("sensors");
const string topicActions("actions");
map<int,bool> statuses;

void delivered(void *contex, MQTTClient_deliveryToken dt) {
	printf("Message with token value %d delivery confirmed\n", dt);
}

int messageArrived(void *context, char *topicName, int topicLen,
		MQTTClient_message *message) {
	int i;
	char *payloadPtr;
	MQTTClient client = (MQTTClient)context;
	printf("Message arrived %s on topic %s\n", (char*) message->payload,
			topicName);
	payloadPtr = (char *)message->payload;
	printf("Real message: ");
	for (int i = 0; i < message->payloadlen; i++) {
		putchar(*payloadPtr++);
	}
	putchar('\n');

	string topics(topicName);
	int pos = topics.find_first_of("/");
	int topicNr = atoi(topics.substr(pos + 1,topics.length()).c_str());
	string replyTopic(topicActions);
	replyTopic += "/";
	replyTopic += to_string(topicNr);
	MQTTClient_deliveryToken token;
	MQTTClient_message pubMsg = MQTTClient_message_initializer;

	std::map<int,bool>::iterator it;
	it = statuses.find(topicNr);
	if (it->second) {
		pubMsg.payload = (void *)"0";
		it->second = false;
	} else {
		pubMsg.payload = (void *)"1";
		it->second = true;
	}
	if (it == statuses.end()) {
		bool status = true;
		pubMsg.payload = (void *)"1";
		statuses.insert(pair<int,bool>(topicNr,status));
	}
	pubMsg.payloadlen = 1;
	pubMsg.qos = QOS;
	pubMsg.retained = 0;
	MQTTClient_publishMessage(client, replyTopic.c_str(), &pubMsg, &token);
	printf("Message sent %s to topic %s\n",pubMsg.payload,replyTopic.c_str());
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
	MQTTClient_setCallbacks(client, client, connLost, messageArrived, delivered);
	if ((rc = MQTTClient_connect(client,&connOps)) != MQTTCLIENT_SUCCESS) {
		printf("Failed to connect return code %d\n",rc);
		exit(EXIT_FAILURE);
	}
	string realTopicSensors = topicSensors;
	realTopicSensors.append("/#");
	printf("Subscribing to topic %s for client %s using Qos=%d\n",realTopicSensors.c_str(),CLIENTID,QOS);
	printf("Press Q<Enter> to quit\n\n");
	MQTTClient_subscribe(client, realTopicSensors.c_str(), QOS);
	do {
		ch = getchar();
	} while(ch !='Q' && ch != 'q');
	MQTTClient_disconnect(client, TIMEOUT);
	MQTTClient_destroy(&client);
	return rc;
}
