CC=gcc

IDIR =../include
ODIR=./obj

CFLAGS=-I $(IDIR) -Wall
LIBS=-lm

EXEC = mystation bus station-manager

_DEPS = utility.h mystationUtility.h sharedSemaphores.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_MYSORT_OBJ = mystation.o utility.o mystationUtility.o
MYSORT_OBJ = $(patsubst %,$(ODIR)/%,$(_MYSORT_OBJ))

_BUS_OBJ = bus.o utility.o
BUS_OBJ = $(patsubst %,$(ODIR)/%,$(_BUS_OBJ))

_STATION_MANAGER_OBJ = station-manager.o utility.o
STATION_MANAGER_OBJ = $(patsubst %,$(ODIR)/%,$(_STATION_MANAGER_OBJ))


$(ODIR)/%.o: %.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(EXEC)

mystation: $(MYSORT_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

bus: $(BUS_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

station-manager: $(STATION_MANAGER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

$(ODIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(ODIR)/
	rm -f $(EXEC)
