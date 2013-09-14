
LFLAGS =-lssl -lm -lssl -lcrypto

#Uso
#./peerClient <trackerHost> <trackerPort> <clientPort> <consolePort>
peerClient: fileHelper.o clientItem.o peerClient.o util.o
	g++ -Wall peerClient.o clientItem.o fileHelper.o util.o -o client $(LFLAGS)

peerClient.o: peerClient.cc
	g++ -c peerClient.cc
#Uso
#./peerTracker <puerto>
peerTracker: fileHelper.o clientItem.o peerTracker.o
	g++ -Wall peerTracker.o clientItem.o fileHelper.o -o tracker $(LFLAGS)

clientItem.o: clientItem.cc clientItem.hh
	g++ -c clientItem.cc
fileHelper.o: fileHelper.cc fileHelper.hh
	g++ -c fileHelper.cc 
peerTracker.o: peerTracker.cc
	g++ -c peerTracker.cc

util.o: util.cc util.hh
	g++ -c util.cc


#/libssl.a
all:
	make peerClient
	make peerTracker

clean:
	rm *.o -f
	rm *.out -f
	rm server -f
	rm client -f
	rm peer -f
	rm peerClient -f
	rm peerTracker -f	
	rm tracker -f
	rm *~ -f
	rm *.TMP -f
	rm *.tmp -f

rebuild:
	make clean
	make all