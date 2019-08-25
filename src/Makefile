CC=g++
CFLAGS = -Wall -pthread -fPIC -O2 -std=c++11  
# CFLAGS = -Wall -pthread -fPIC -g -std=c++11 

EXE = cada0030
OBJ = main.o tdm.o component.o

all:$(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o ../$@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f *.o ../$(EXE)                