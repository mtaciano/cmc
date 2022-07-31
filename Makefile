# Makefile para o compilador C-
# Miguel Silva Taciano e Gabriel Bianchi e Silva

CC = gcc
CC-FLAGS = -g
BISON = bison
LEX = flex
RS = cargo
RS-FLAGS = --release
TARGET = target/release

BIN = compilador
BIN-FLAGS = -pthread -ldl

OBJS = cmin.tab.o lex.yy.o main.o util.o symtab.o analyze.o code.o librust.a

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(BIN-FLAGS) -o $(BIN)

main.o: main.c globals.h util.h scan.h analyze.h
	$(CC) $(CC-FLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CC-FLAGS) -c util.c

code.o: code.c code.h globals.h
	$(CC) $(CC-FLAGS) -c code.c

symtab.o: symtab.c symtab.h
	$(CC) $(CC-FLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CC-FLAGS) -c analyze.c

lex.yy.o: cmin.l scan.h util.h globals.h
	$(LEX) -o lex.yy.c cmin.l
	$(CC) $(CC-FLAGS) -c lex.yy.c

cmin.tab.o: cmin.y globals.h
	$(BISON) -d cmin.y
	$(CC) $(CC-FLAGS) -c cmin.tab.c

librust.a: rust/src/lib.rs rust/Cargo.toml rust/wrapper.h rust/src/assembly.rs rust/src/binary.rs code.h globals.h
	cd rust && $(RS) build $(RS-FLAGS)
	cd rust && mv $(TARGET)/librust.a ../librust.a

bindings:
	cd rust && cbindgen --config cbindgen.toml --crate rust --output ../rust.h

debug: RS-FLAGS =
debug: TARGET = target/debug
debug: $(BIN)

# Só rodar se a pasta existir
cpu:
	cp out_bin.txt ../processador/out_bin.txt

clean:
	-rm -f $(BIN)
	-rm -f *.gv
	-rm -f *.tab.c
	-rm -f *.tab.h
	-rm -f *.yy.c
	-rm -f out_bin.txt
	-rm -r rust/target/release/*
	-rm -r rust/target/debug/*
	-rm -f $(OBJS)
