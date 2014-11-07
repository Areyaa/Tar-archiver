CC = gcc
CFLAGS = -Wall -lm
SRC = my_tar.c
EXE = my_tar
 
all:
	$(CC) -o $(EXE) $(SRC) $(CFLAGS)
 

.PHONY : clean
clean :
	rm -f $(EXE) *~
