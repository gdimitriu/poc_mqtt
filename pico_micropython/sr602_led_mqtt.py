from machine import Pin
from time import sleep
import network
from MQTT.MQTTClient import MQTTClient
import config
import _thread

stop_thread = False
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


def connect_mqtt():
    try:
        client = MQTTClient(client_id=MQTT_CLIENT_ID,
                            server=MQTT_SERVER,
                            port=MQTT_PORT,
                            user=MQTT_USER,
                            password=MQTT_PASSWORD,
                            keepalive=MQTT_KEEPALIVE)
        client.connect()
        return client
    except Exception as e:
        print('Error connecting to MQTT:', e)
        raise  # Re-raise the exception to see the full traceback


def publish_mqtt(topic, value):
    global client
    client.publish(topic, value)
    print(topic)
    print(value)
    print("Publish Done")


interrupt_flag = 0
pin = Pin(5, Pin.IN, Pin.PULL_UP)
led = Pin(15, Pin.OUT)
led.low()


def callback_sr602(pin):
    global interrupt_flag
    interrupt_flag = 1


pin.irq(trigger=Pin.IRQ_RISING, handler=callback_sr602)

MQTT_TOPIC_SENSOR = "sensors/sr602"
MQTT_TOPIC_LED_OFF = b"sensors/led/off"


def callback_led(topic, msg):
    print('Received message:', topic, msg)
    if MQTT_TOPIC_LED_OFF == topic:
        led.low()

def monitor_sr602():
    global interrupt_flag
    global stop_thread
    while True:
        if interrupt_flag is 1:
            interrupt_flag = 0
            led.high()
            publish_mqtt(MQTT_TOPIC_SENSOR, "intruder detected")
        if stop_thread:
            break


def run():
    global interrupt_flag
    global client
    global stop_thread
    try:
        if not initialize_wifi(config.wifi_ssid, config.wifi_password):
            print('Error connecting to the network... exiting program')
        else:
            # Connect to MQTT broker, start MQTT client
            client = connect_mqtt()
            client.set_callback(callback_led)
            client.subscribe(MQTT_TOPIC_LED_OFF)
            _thread.start_new_thread(monitor_sr602, ())
            while True:
                client.wait_msg()

    except Exception as e:
        print('Error:', e)
        stop_thread = True
