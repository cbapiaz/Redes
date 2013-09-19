
LFLAGS =-lssl -lm -lssl -lcrypto

#Uso
#./Client <trackerHost> <trackerPort> <clientPort> <consolePort>
client: fileHelper.o clientItem.o client.o util.o
	g++ -Wall Client.o clientItem.o fileHelper.o util.o -o client $(LFLAGS)

client.o: client.cc
	g++ -c client.cc
#Uso
#./Tracker <puerto>
tracker: fileHelper.o clientItem.o tracker.o util.o
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
	make client
	make tracker

clean:
	rm *.o -f
	rm *.out -f
	rm server -f
	rm client -f
	rm peer -f
	rm client -f	
	rm tracker -f
	rm *~ -f
	rm *.TMP -f
	rm *.tmp -f
	rm *.stackdump -f

rebuild:
	make clean
	make all