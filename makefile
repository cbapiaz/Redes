server:
	g++ server.cc -o server
client:
	g++ client.cc -o client
all:
	make server
	make client

clean:
	rm *.o -f
	rm *.out -f
	rm server -f
	rm client -f
	rm *~ -f
