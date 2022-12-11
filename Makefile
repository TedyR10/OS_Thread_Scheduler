build: libscheduler.so

libscheduler.so: so_scheduler.o queue.o
	gcc -Wall -shared so_scheduler.o queue.o -o libscheduler.so

so_scheduler.o: so_scheduler.c
	gcc -Wall -fPIC -o so_scheduler.o -c -lpthread so_scheduler.c

queue.o: queue.c
	gcc -Wall -fPIC -o queue.o -c queue.c

.PHONY: clean

clean:
	rm -f *.o libscheduler.so
