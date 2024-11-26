CC=gcc
CFLAGS=-g -Wall -pthread

SRCDIR=src
INCDIR=include
LIBDIR=lib
OUTPUTDIR=output/img

all: outdir utils server client

utils: 
	$(CC) $(CFLAGS) -c $(SRCDIR)/utils.c -o $(LIBDIR)/utils.o
server: $(LIBDIR)/server.o $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

client: $(LIBDIR)/client.o $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

.PHONY: clean outdir

clean:
	rm -f server client $(LIBDIR)/utils.o
	rm -rf $(OUTPUTDIR)
	rm -f server_log

outdir:
	mkdir -p $(OUTPUTDIR)
