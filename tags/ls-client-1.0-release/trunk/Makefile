libclient: src/*.cpp include/*.h
	g++ -g -Wall -fPIC -shared -D__CPLUSPLUS -DDEBUG_EPEX -I../lsnet/include -Iinclude src/*.cpp ../lsnet/src/*.c -o libclient.so -lpthread
client: test/main.cpp
	g++ -g -Wall -D__CPLUSPLUS -DDEBUG_EPEX -I../lsnet/include -Iinclude src/*.cpp test/main.cpp ../lsnet/src/*.c -o client -lpthread
client2: test/push.cpp
	g++ -g -Wall -D__CPLUSPLUS -DLOG_LEVEL=2 -I../lsnet/include -Iinclude src/*.cpp test/push.cpp ../lsnet/src/*.c -o client2 -lpthread
clean:
	rm -rf libclient.so
	rm -rf client
	rm -rf client2
