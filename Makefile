client: src/*.cpp include/*.h
	g++ -g -Wall -D__CPLUSPLUS -I../lsnet/include -Iinclude -lpthread src/*.cpp ../lsnet/src/*.c -o client
clean:
	rm client
