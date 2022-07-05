# Makefile para o compilador C-
# Miguel Silva Taciano e Gabriel Bianchi e Silva

CC = gcc
BISON = bison
LEX = flex
RS = cargo
RS-FLAGS = --all-features --format-version=1 --manifest-path=rust/Cargo.toml

BIN = compilador

OBJS = cmin.tab.o lex.yy.o main.o util.o symtab.o analyze.o code.o librust.a

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN)

main.o: main.c globals.h util.h scan.h analyze.h
	$(CC) -g -c main.c

util.o: util.c util.h globals.h
	$(CC) -g -c util.c

code.o: code.c code.h globals.h
	$(CC) -g -c code.c

symtab.o: symtab.c symtab.h
	$(CC) -g -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) -g -c analyze.c

lex.yy.o: cmin.l scan.h util.h globals.h
	$(LEX) -o lex.yy.c cmin.l
	$(CC) -g -c lex.yy.c

cmin.tab.o: cmin.y globals.h
	$(BISON) -d cmin.y
	$(CC) -g -c cmin.tab.c

librust.a: rust/src/lib.rs rust/Cargo.toml rust/wrapper.h code.h globals.h
	cd rust && $(RS) build --release
	cd rust && mv target/release/librust.a ../librust.a

bindings:
	cd rust && cbindgen --config cbindgen.toml --crate rust --output ../rust.h

clean:
	-rm -f $(BIN)
	-rm -f *.gv
	-rm -f *.tab.c
	-rm -f *.tab.h
	-rm -f *.yy.c
	-rm -f $(OBJS)
