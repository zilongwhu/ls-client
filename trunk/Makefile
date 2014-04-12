all: libclient.a client client2

libclient.a: src/client.o src/net_poller.o src/net_proxy.o src/net_utils.o src/server.o src/service.o src/service_manager.o src/client_manager.o
	ar crs $@ $^

src/client.o: src/client.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -DLOG_LEVEL=0 -c $^ -o $@
src/net_poller.o: src/net_poller.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -DLOG_LEVEL=0 -c $^ -o $@
src/net_proxy.o: src/net_proxy.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -DLOG_LEVEL=0 -c $^ -o $@
src/net_utils.o: src/net_utils.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -DLOG_LEVEL=0 -c $^ -o $@
src/server.o: src/server.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -DLOG_LEVEL=0 -c $^ -o $@
src/service.o: src/service.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -I../conflib/include -DLOG_LEVEL=0 -c $^ -o $@
src/service_manager.o: src/service_manager.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -I../conflib/include -DLOG_LEVEL=0 -c $^ -o $@
src/client_manager.o: src/client_manager.cpp
	g++ -g -Wall -Iinclude -I../lsnet/include -I../conflib/include -DLOG_LEVEL=0 -c $^ -o $@

client: test/main.cpp libclient.a
	g++ -g -Wall -Iinclude -I../lsnet/include test/main.cpp -Xlinker "-(" libclient.a ../lsnet/liblsnet.a ../conflib/libconf.a -Xlinker "-)" -o client -lpthread
client2: test/push.cpp libclient.a
	g++ -g -Wall -Iinclude -I../lsnet/include test/push.cpp -Xlinker "-(" libclient.a ../lsnet/liblsnet.a ../conflib/libconf.a -Xlinker "-)" -o client2 -lpthread

clean:
	rm -rf libclient.a
	rm -rf client
	rm -rf client2
	rm -rf src/*.o
