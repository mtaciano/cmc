
# Makefile for C-Minus compiler
# @trilliwon

CC = gcc
BISON = bison
LEX = flex

BIN = compiler

OBJS = cmin.tab.o lex.yy.o main.o util.o # symtab.o analyze.o

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN)

main.o: main.c globals.h util.h scan.h # analyze.h
	$(CC) -c main.c

util.o: util.c util.h globals.h
	$(CC) -c util.c

# symtab.o: symtab.c symtab.h
# 	$(CC) -c symtab.c

# analyze.o: analyze.c globals.h symtab.h analyze.h
# 	$(CC) -c analyze.c

lex.yy.o: cmin.l scan.h util.h globals.h
	$(LEX) -o lex.yy.c cmin.l
	$(CC) -c lex.yy.c

cmin.tab.o: cmin.y globals.h
	$(BISON) -d cmin.y
	$(CC) -c cmin.tab.c

clean:
	-rm -f $(BIN)
	-rm -f cmin.tab.c
	-rm -f cmin.tab.h
	-rm -f lex.yy.c
	-rm -f $(OBJS)
