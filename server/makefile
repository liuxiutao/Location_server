location: calculation.o communication main.o
		g++ -o location calculation.o communication.o main.o


calculation.o: location.h calculation.cc
		g++ -c calculation.cc

communication.o: communication.cc location.h
		g++ -c communication.cc

main.o: main.cc location.h
		g++ -c main.cc -lfcgi -lpthread

clear:
		rm *.o
