package io.github.gdimitriu.poc_mqtt;

import org.eclipse.paho.mqttv5.client.IMqttMessageListener;
import org.eclipse.paho.mqttv5.client.IMqttToken;
import org.eclipse.paho.mqttv5.client.MqttClient;
import org.eclipse.paho.mqttv5.client.MqttConnectionOptions;
import org.eclipse.paho.mqttv5.client.persist.MemoryPersistence;
import org.eclipse.paho.mqttv5.common.MqttException;
import org.eclipse.paho.mqttv5.common.MqttMessage;
import org.eclipse.paho.mqttv5.common.MqttSubscription;

import java.util.HashMap;
import java.util.Map;

public class ButtonLedMqtt implements IMqttMessageListener {
    private int qos = 1;
    private String server = "localhost";
    private String port = "1883";
    private String clientId = "MQTT_subscriber_ButtonLedMqtt";
    private MemoryPersistence persistence = new MemoryPersistence();
    private MqttClient client;
    private String topicSensors ="sensors";
    private String topicActions ="actions";
    private String user = "mqtt";
    private String password = "mqtt";
    private Map<Integer,Boolean> statusMap;

    public ButtonLedMqtt() {
        statusMap = new HashMap<>();
    }

    public void setServer(String server) {
        this.server = server;
    }

    public void setPort(String port) {
        this.port = port;
    }

    public void setTopicSensors(String topicSensors) {
        this.topicSensors = topicSensors;
    }

    public void setTopicActions(String topicActions) {
        this.topicActions = topicActions;
    }

    public void setUser(String user) {
        this.user = user;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public void connect() throws MqttException {
        String broker = "tcp://" + server + ":" + port;
        try {
            System.out.println("Start connecting to MQTT broker: " + broker);
            MqttConnectionOptions connOpts = new MqttConnectionOptions();
            connOpts.setCleanStart(false);
            connOpts.setPassword(user.getBytes());
            connOpts.setUserName(password);
            client = new MqttClient(broker,clientId, persistence);
            client.connect(connOpts);
            System.out.println("End Connecting.");
        } catch (MqttException e) {
            System.out.println("reason=" + e.getReasonCode());
            System.out.println("message=" + e.getMessage());
            System.out.println("localized message=" + e.getLocalizedMessage());
            System.out.println("cause=" + e.getCause());
            e.printStackTrace();
            throw e;
        }
    }

    public void listen() throws Exception {
        try {
            //subscribe to all sensors
            System.out.println("Subscribing to topic " + topicSensors + "/#");

            MqttSubscription sub =
                    new MqttSubscription(topicSensors + "/#", qos);

            IMqttToken token = client.subscribe(
                    new MqttSubscription[] { sub },
                    new IMqttMessageListener[] { this });
        }
        catch ( Exception e ) {
            e.printStackTrace();
        }
    }
    @Override
    public void messageArrived(String topic, MqttMessage mqttMessage) throws Exception {
        String messageText = new String(mqttMessage.getPayload());
        System.out.println("Message received " + messageText + " on topic " + topic);
        //find the sensor number from the topic
        //arduino does not send correlation Id or response topic so we have to find from the topic name
        Integer sensorNr = Integer.valueOf(topic.substring(topic.indexOf('/') + 1, topic.length()));
        String responseTopic = topicActions + '/' + sensorNr;
        System.out.println("Send back the action to the topic " + responseTopic);
        // send back action to the client
        MqttMessage response = new MqttMessage();
        Boolean status = statusMap.get(sensorNr);
        String payload;
        if (status == null) {
            status = true;
            payload = "1";
            statusMap.put(sensorNr,status);
        } else {
            status = !status;
            payload = status ? "1" : "0";
            statusMap.replace(sensorNr,status);
        }
        response.setPayload(payload.getBytes());
        client.publish(responseTopic,response);
    }

    public static void main(String[] args) {
        ButtonLedMqtt subscriber = new ButtonLedMqtt();
        if ( args.length > 0) {
            for ( int i = 0; i < args.length; i++ ) {
                switch ( args[i] ) {
                    case "server":
                        subscriber.setServer(args[++i]);
                        break;
                    case "port":
                        subscriber.setPort(args[++i]);
                        break;
                    case "user":
                        subscriber.setUser(args[++i]);
                        break;
                    case "password":
                        subscriber.setPassword(args[++i]);
                        break;
                    case "topicSensorsPrefix":
                        subscriber.setTopicSensors(args[++i]);
                        break;
                    case "topicActionsPrefix":
                        subscriber.setTopicActions(args[++i]);
                        break;
                }
            }
        }

        try {
            subscriber.connect();
            subscriber.listen();
        }
        catch ( Exception e ) {
            e.printStackTrace();
        }
    }
}
