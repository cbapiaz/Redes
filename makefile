
LFLAGS =-lssl -lm -lssl -lcrypto

#Uso
#./Client <trackerHost> <trackerPort> <clientPort> <consolePort>
Client: fileHelper.o clientItem.o Client.o util.o
	g++ -Wall Client.o clientItem.o fileHelper.o util.o -o client $(LFLAGS)

Client.o: Client.cc
	g++ -c Client.cc
#Uso
#./Tracker <puerto>
Tracker: fileHelper.o clientItem.o Tracker.o util.o
	g++ -Wall Tracker.o clientItem.o fileHelper.o util.o -o tracker $(LFLAGS)

clientItem.o: clientItem.cc clientItem.hh
	g++ -c clientItem.cc
fileHelper.o: fileHelper.cc fileHelper.hh
	g++ -c fileHelper.cc 
Tracker.o: Tracker.cc
	g++ -c Tracker.cc

util.o: util.cc util.hh
	g++ -c util.cc


#/libssl.a
all:
	make Client
	make Tracker

clean:
	rm *.o -f
	rm *.out -f
	rm server -f
	rm client -f
	rm peer -f
	rm Client -f
	rm Tracker -f	
	rm tracker -f
	rm *~ -f
	rm *.TMP -f
	rm *.tmp -f

rebuild:
	make clean
	make all