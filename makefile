CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp webserver.cpp ./timer/time_wheel.cpp 
	$(CXX) -g -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm  -r server
