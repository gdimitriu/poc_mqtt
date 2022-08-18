# poc_mqtt
MQTT usage for different OS and microcontrollers

The button_led_mqtt is my interpretation of : https://www.instructables.com/How-to-Use-MQTT-With-the-Raspberry-Pi-and-ESP8266/

The java project button_led_mqtt is the java application to interact with the mqtt from ESP8266.

If it receives button pressed operation then it will toggle the LED on the ESP8266 on the specific device that send the command.

The sensor number is found from the subscribing topic for example: sensors/1 or sensors/2 it sends command to topic actions/1 or actions/2.

For the java version please look at : https://blogs.oracle.com/javamagazine/post/java-mqtt-iot-message-queuing