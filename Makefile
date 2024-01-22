# Makefile pour le projet Game Of Life en 3D
MPICC = mpicc
CFLAGS = -std=c11 -Wall -Wextra -Werror -Wpedantic -Wconversion
LDFLAGS =
EXEC = main
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

ifeq ($(OS), Windows_NT)
	DEL = del /q /s
else
	DEL = rm -rf
endif

RESULTS_FOLDER = results
ifeq ($(OS), Windows_NT)
	ALL_RESULTS = .\$(RESULTS_FOLDER)\*
else
	ALL_RESULTS = ./$(RESULTS_FOLDER)/*
endif

ifeq ($(OS), Windows_NT)
	DEL_EXEC = $(DEL) $(EXEC).exe
else
	DEL_EXEC = $(DEL) $(EXEC)
endif


all : $(EXEC)

$(EXEC) : $(OBJ)
	$(MPICC) -o $(EXEC) $(OBJ) $(LDFLAGS)

%.o : %.c
	$(MPICC) -o $@ -c $< $(CFLAGS)

clean :
	$(DEL) *.o
	$(DEL) $(ALL_RESULTS)

mrproper : clean
	$(DEL_EXEC)
