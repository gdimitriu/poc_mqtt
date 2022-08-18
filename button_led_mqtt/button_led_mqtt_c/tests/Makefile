CC = gcc
CXX = g++
OBJECTS = 
EXECUTABLES = async_publish async_subscriber async_thsafe_subscriber
LIBS = -lpaho-mqtt3c
FLAGS = -std=gnu++11 -ggdb

all: $(OBJECTS) $(EXECUTABLES)

async_publish: async_publish.c
	$(CC) -fPIC -o async_publish async_publish.c $(LIBS) $(FLAGS)

async_subscriber: async_subscriber.c
	$(CC) -fPIC -o async_subscriber async_subscriber.c $(LIBS) $(FLAGS)
	
async_thsafe_subscriber: async_thsafe_subscriber.c
	$(CC) -fPIC -o async_thsafe_subscriber async_thsafe_subscriber.c -lpaho-mqtt3as $(FLAGS)
	
clean:
	for object in $(EXECUTABLES);\
	do \
		if [ -f $$object ]; then rm $$object; fi; \
	done
	for object in $(OBJECTS);\
	do \
		if [ -f $$object ]; then rm $$object; fi; \
	done