CC=gcc
CFLAGS=-c -Wall -0fast
ODIR=obj/
LDFLAGS=
SOURCES=../src/LZW.c ../src/LZWDecDictionary.c ../src/LZWEnDictionary.c ../src/LZWHash.c ../src/LZWIOHandler.c
OBJS=$(ODIR:.c=.o)
EXECUTABLE=LZW

OBJECTS = $(patsubst %,$(ODIR)/%,$(OBJS))

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	



