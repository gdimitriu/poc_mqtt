from MQTT.MQTTClient import MQTTClient
from machine import Pin
import binascii
import machine
import network
import config
from time import sleep

# ESP8266 ESP-12 modules have blue, active-low LED on GPIO2, replace
# with something else if needed.
led = Pin(15, Pin.OUT)
led.low()

# Default MQTT server to connect to
SERVER = "192.168.1.29"
CLIENT_ID = binascii.hexlify(machine.unique_id())
TOPIC = b"led"


state = 0


def sub_cb(topic, msg):
    global state
    print((topic, msg))
    if msg == b"on":
        led.high()
        state = 1
    elif msg == b"off":
        led.low()
        state = 0
    elif msg == b"toggle":
        # LED is inversed, so setting it to current state
        # value will make it toggle
        led.value(state)
        state = 1 - state


# MQTT Parameters
MQTT_SERVER = config.mqtt_server
MQTT_PORT = 0
MQTT_USER = config.mqtt_username
MQTT_PASSWORD = config.mqtt_password
MQTT_CLIENT_ID = b"sensors_sr602"
MQTT_KEEPALIVE = 7200
MQTT_SSL = False
client = None

def initialize_wifi(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    # Connect to the network
    wlan.connect(ssid, password)

    # Wait for Wi-Fi connection
    connection_timeout = 10
    while connection_timeout > 0:
        if wlan.status() >= 3:
            break
        connection_timeout -= 1
        print('Waiting for Wi-Fi connection...')
        sleep(1)

    # Check if connection is successful
    if wlan.status() != 3:
        return False
    else:
        print('Connection successful!')
        network_info = wlan.ifconfig()
        print('IP address:', network_info[0])
        return True


def main(server=SERVER):
    try:
        if not initialize_wifi(config.wifi_ssid, config.wifi_password):
            print('Error connecting to the network... exiting program')
        else:
            c = MQTTClient(CLIENT_ID, server, user="mqtt", password="mqtt")
            # Subscribed messages will be delivered to this callback
            c.set_callback(sub_cb)
            c.connect()
            c.subscribe(TOPIC)
            print("Connected to %s, subscribed to %s topic" % (server, TOPIC))

            try:
                while 1:
                    # micropython.mem_info()
                    c.wait_msg()
            finally:
                c.disconnect()
    except Exception as e:
        print('Error:', e)
