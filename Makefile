# Tool invocations
CC=g++
SRC=src
CFLAGS=-I$(SRC) -std=c++11 -O3 -Wall -fmessage-length=0

ODIR=Release
LIBS=-lpthread

_OBJS = BasicHTTP CmdLine Config DataHandler DataHandler/Exec DataHandler/Static Logger LoggingSingleton Manager Router SignalHandler Worker
OBJ = $(patsubst %,$(ODIR)/%.o,$(_OBJS))

hackttp: $(OBJ) $(ODIR)/server.o 
	@echo 'Building HackTTP: $@'
	@$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


$(ODIR)/DataHandler/%.o: $(SRC)/DataHandler/%.cpp $(SRC)/DataHandler.h $(ODIR)/.empty
	@echo 'Building object: $@'
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SRC)/%.cpp $(SRC)/%.h $(ODIR)/.empty
	@echo 'Building object: $@'
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/server.o: $(SRC)/server.cpp
	@echo 'Building object: $@'
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/.empty:
	@echo 'Creating $(ODIR) directory'
	@mkdir -p $(ODIR)/DataHandler
	@touch $(ODIR)/.empty

all: hackttp

clean:
	@echo 'Removing: hackttp $(ODIR)'
	@rm -rf hackttp $(ODIR)

.PHONY: all clean
