all: echoserver

echoserver:
	g++ -pthread src/baseserver.cc src/echoserver.cc echodriver.cc -I include -o $@
