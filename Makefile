SOURCES = main.c
LIBS = c
LIBDIRS = 
INCDIRS = 
EXECUTABLE = impractical-web-server

CC = gcc
CFLAGS = -g -O2 -mdynamic-no-pic -std=gnu99
LDFLAGS = $(addprefix -L,$(LIBDIRS)) $(addprefix -l,$(LIBS))
IFLAGS = -I. $(addprefix -I,$(INCDIRS))
DEFS = 
WFLAGS = -W -Wall -Wwrite-strings -Wc++-compat -Wstrict-prototypes -pedantic
OUTPUT_OPTION = -o $@

OBJECTS=$(SOURCES:.c=.o)
COMPILE.c = $(CC) -c $(DEFS) $(CFLAGS) -c $(IFLAGS) $(WFLAGS)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(OUTPUT_OPTION)

depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) $(IFLAGS) -MM $^>>./.depend;

include .depend

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
