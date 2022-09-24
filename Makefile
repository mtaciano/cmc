# Makefile para o compilador C-
# Miguel Silva Taciano e Gabriel Bianchi e Silva

# C/C++
CC = clang
CC-FLAGS = -Wall -Wextra -pedantic

# Scanner e Parser
BISON = bison
BISON-FLAGS = -W --color=always -d
LEX = flex

# Rust
RS = cargo
RS-FLAGS =
TARGET = target/debug

# C Minus Compiler
BIN = cmc
BIN-FLAGS = -static

# Componentes do compilador
OBJS = parse.tab.o scan.yy.o main.o util.o symtab.o analyze.o code.o librust.a

# Executável
$(BIN): $(OBJS)
	$(CC) $(OBJS) $(CC-FLAGS) $(BIN-FLAGS) -o $(BIN)

# Otimização de velocidade e tamanho
release: CC-FLAGS += -O3
release: RS-FLAGS += --release
release: TARGET = target/release
release: $(BIN)
release:
	strip -p --strip-all $(BIN)

# Símbolos de debug
debug: CC-FLAGS += -g -fsanitize=address,undefined -fno-omit-frame-pointer
debug: BISON-FLAGS += -g
debug: BIN-FLAGS =
debug: $(BIN)

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

scan.yy.o: scan.l scan.h util.h globals.h
	$(LEX) -o scan.yy.c scan.l
	$(CC) $(CC-FLAGS) -c scan.yy.c

parse.tab.o: parse.y globals.h
	$(BISON) $(BISON-FLAGS) parse.y
	$(CC) $(CC-FLAGS) -c parse.tab.c

.ONESHELL:
librust.a: rust/src/lib.rs rust/Cargo.toml rust/wrapper.h rust/src/assembly.rs rust/src/binary.rs code.h globals.h
	cd rust
	$(RS) build $(RS-FLAGS)
	mv $(TARGET)/librust.a ../librust.a

# Gera a FFI entre C e Rust
bindings:
	cd rust && cbindgen --config cbindgen.toml --crate rust --output ../rust.h

# Só rodar se a pasta existir
# Copia o binário para o processador
cpu:
	cp out_bin.txt ../processador/out_bin.txt

clean:
	-rm -f $(BIN)
	-rm -f *.gv
	-rm -f *.tab.c
	-rm -f *.tab.h
	-rm -f *.yy.c
	-rm -f out_bin.txt
	-rm -r rust/target/
	-rm -f $(OBJS)
