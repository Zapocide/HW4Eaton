TARGET = server
LIBS = -lm -lpthread
CC = gcc
CPP = g++
CFLAGS = -g -Wall

.PHONY: all clean

all: $(TARGET)

COBJECTS = server.o
CHEADERS = server.h
CPPOBJECTS = database.cpp
CPPHEADERS = database.h

%.o: %.c $(CHEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp $(CPPHEADERS)
	$(CPP) $(CFLAGS) -c $< -o $@

$(TARGET): $(COBJECTS) $(CPPOBJECTS)
	$(CPP) $(COBJECTS) $(CPPOBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)