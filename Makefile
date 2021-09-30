CFLAGS = -std=gnu99 -w -m32 -g -D_GNU_SOURCE
LIBS = -lpthread -pthread
SOURCES1 = app1.c
SOURCES2 = app2.c
SOURCES3 = app3.c
SOURCES4 = app4.c
SOURCES5 = app5.c
SOURCES0 = psu_thread.c
OUT1 = app1
OUT2 = app2
OUT3 = app3
OUT4 = app4
OUT5 = app5

default:
	gcc $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT4) $(SOURCES4) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT5) $(SOURCES5) $(SOURCES0)
debug:
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES0)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES0)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES0)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT4) $(SOURCES4) $(SOURCES0)
	gcc -g $(CFLAGS) $(LIBS) -o $(OUT5) $(SOURCES5) $(SOURCES0)
all:
	gcc $(CFLAGS) $(LIBS) -o $(OUT1) $(SOURCES1) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT2) $(SOURCES2) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT3) $(SOURCES3) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT4) $(SOURCES4) $(SOURCES0)
	gcc $(CFLAGS) $(LIBS) -o $(OUT5) $(SOURCES5) $(SOURCES0)
clean:
	rm $(OUT1) $(OUT2) $(OUT3) $(OUT4) $(OUT5)
