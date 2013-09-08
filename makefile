#Uso
#./peerClient <host> <trackerPort> <clientPort> <consolePort>
peerClient:
	g++ peerClient.cc -o peerClient
#Uso
#./peerTracker <puerto>
peerTracker:
	g++ peerTracker.cc clientItem.cc clientItem.hh -o peerTracker


p2p:
	make peerClient
	make peerTracker

#server, client, peer de prueba
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
	rm peerClient -f
	rm peerTracker -f
	rm *~ -f
	rm *.TMP -f
	rm *.tmp -f

rebuild:
	make clean
	make all