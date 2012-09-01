client: src/*.c
	gcc -g -Wall -I../lsnet/include -llsnet -L../lsnet -lpthread src/*.c -o client
clean:
	rm client
