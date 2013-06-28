client: src/*.cpp include/*.h
	g++ -g -Wall -D__CPLUSPLUS -I../lsnet/include -Iinclude src/*.cpp ../lsnet/src/*.c -o client -lpthread
clean:
	rm client
