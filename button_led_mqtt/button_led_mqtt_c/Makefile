CC = gcc
CXX = g++
OBJECTS = 
EXECUTABLES = button_led_mqtt
LIBS = -lpaho-mqtt3c
FLAGS = -std=gnu++11 -ggdb

all: $(OBJECTS) $(EXECUTABLES)
	cd tests && $(MAKE)
	
button_led_mqtt: button_led_mqtt.cpp
	$(CXX) -fPIC -o button_led_mqtt button_led_mqtt.cpp $(LIBS) $(FLAGS)
	
clean:
	for object in $(EXECUTABLES);\
	do \
		if [ -f $$object ]; then rm $$object; fi; \
	done
	for object in $(OBJECTS);\
	do \
		if [ -f $$object ]; then rm $$object; fi; \
	done
	cd tests && $(MAKE) clean