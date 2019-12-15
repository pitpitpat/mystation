CC=gcc

IDIR =../include
ODIR=./obj

CFLAGS=-I $(IDIR) -Wall
LIBS=-lpthread

EXEC = mystation station-manager bus comptroller

_DEPS = utility.h sharedMemory.h mystationUtility.h stationManagerUtility.h busUtility.h comptrollerUtility.h isleInfo.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_MY_STATION_OBJ = mystation.o utility.o sharedMemory.o mystationUtility.o isleInfo.o
MY_STATION_OBJ = $(patsubst %,$(ODIR)/%,$(_MY_STATION_OBJ))

_STATION_MANAGER_OBJ = station-manager.o utility.o sharedMemory.o stationManagerUtility.o isleInfo.o
STATION_MANAGER_OBJ = $(patsubst %,$(ODIR)/%,$(_STATION_MANAGER_OBJ))

_BUS_OBJ = bus.o utility.o sharedMemory.o busUtility.o
BUS_OBJ = $(patsubst %,$(ODIR)/%,$(_BUS_OBJ))

_COMPTROLLER_OBJ = comptroller.o utility.o sharedMemory.o comptrollerUtility.o isleInfo.o
COMPTROLLER_OBJ = $(patsubst %,$(ODIR)/%,$(_COMPTROLLER_OBJ))


$(ODIR)/%.o: %.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(EXEC)

mystation: $(MY_STATION_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

station-manager: $(STATION_MANAGER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

bus: $(BUS_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

comptroller: $(COMPTROLLER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Executable file \"$@\" created";

$(ODIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(ODIR)/
	rm -f $(EXEC)
