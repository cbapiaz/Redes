server:
	g++ server.cc -o server
client:
	g++ client.cc -o client
peer:
	g++ peer.cc -o peer

all:
	make server
	make client
	make peer

clean:
	rm *.o -f
	rm *.out -f
	rm server -f
	rm client -f
	rm peer -f
	rm *~ -f
	rm *.TMP -f
	rm *.tmp -f

rebuild:
	make clean
	make all