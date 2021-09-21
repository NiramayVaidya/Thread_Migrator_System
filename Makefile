CFLAGS = -std=gnu99 -w
LIBS = -lpthread -pthread
SOURCES1 = app1.c
SOURCES2 = app2.c
SOURCES3 = app3.c
SOURCES4 = psu_thread.c
OUT1 = app1
OUT2 = app2
OUT3 = app3

default:
	gcc $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES4)
	gcc $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES4)
	gcc $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES4)
debug:
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES4)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES4)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES4)
all:
	gcc $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES4)
	gcc $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES4)
	gcc $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES4)
clean:
	rm $(OUT1) $(OUT2) $(OUT3)
