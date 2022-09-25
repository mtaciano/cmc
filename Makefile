# Makefile para o compilador C-
# Miguel Silva Taciano e Gabriel Bianchi e Silva

# Cores
BOLD := $(shell tput bold)
NORMAL := $(shell tput sgr0)
GREEN=$(shell echo -e "\033[0;32m")
NC=$(shell echo -e "\033[0m") # No color

# Pastas
SRC = src
COMMON = src/common
TARGET = build
RS-TARGET = #empty

# C/C++
CC = clang
CC-FLAGS = -Wall -Wextra -pedantic

# Bison e Flex
BISON = bison
BISON-FLAGS = -W --color=always -d
LEX = flex

# Rust
RS = cargo
RS-FLAGS = #empty

# C Minus Compiler
BIN = cmc
BIN-FLAGS = -static

# Componentes do compilador
OBJS = $(TARGET)/parse.tab.o $(TARGET)/scan.yy.o $(TARGET)/main.o $(TARGET)/util.o $(TARGET)/symtab.o $(TARGET)/analyze.o $(TARGET)/code.o $(TARGET)/librust.a


# Otimização de velocidade e tamanho
.PHONY: release
release: CC-FLAGS += -O3
release: RS-FLAGS += --release
release: RS-TARGET = $(TARGET)/release
release: $(BIN)
release:
	strip -p --strip-all $(TARGET)/$(BIN)


# Símbolos de debug
.PHONY: debug
debug: CC-FLAGS += -g -fsanitize=address,undefined -fno-omit-frame-pointer
debug: BISON-FLAGS += -g
debug: BIN-FLAGS = #empty
debug: RS-TARGET = $(TARGET)/debug
debug: $(BIN)


# Executável
$(BIN): $(OBJS)
	@printf "$(BOLD)$(GREEN)Compilando:$(NC)$(NORMAL) $(BIN)\n"
	$(CC) $(OBJS) $(CC-FLAGS) $(BIN-FLAGS) -o $(TARGET)/$(BIN)
	ln -sf $(TARGET)/$(BIN) $(BIN)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"


# Objetos
$(TARGET)/main.o: $(SRC)/main.c $(COMMON)/globals.h $(COMMON)/util.h $(SRC)/scan.h $(SRC)/analyze.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) main.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/main.c -o $(TARGET)/main.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/util.o: $(COMMON)/util.c $(COMMON)/util.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) util.o\n"
	$(CC) $(CC-FLAGS) -c $(COMMON)/util.c -o $(TARGET)/util.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/code.o: $(SRC)/code.c $(SRC)/code.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) code.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/code.c -o $(TARGET)/code.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/symtab.o: $(SRC)/symtab.c $(SRC)/symtab.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) symtab.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/symtab.c -o $(TARGET)/symtab.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/analyze.o: $(SRC)/analyze.c $(COMMON)/globals.h $(SRC)/symtab.h $(SRC)/analyze.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) analyze.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/analyze.c -o $(TARGET)/analyze.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/scan.yy.o: $(SRC)/scan.l $(SRC)/scan.h $(COMMON)/util.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) scan.yy.o\n"
	$(LEX) -o $(TARGET)/scan.yy.c $(SRC)/scan.l
	$(CC) $(CC-FLAGS) -c $(TARGET)/scan.yy.c -o $(TARGET)/scan.yy.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/parse.tab.o: $(SRC)/parse.y $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) parse.tab.o\n"
	$(BISON) $(BISON-FLAGS) $(SRC)/parse.y -o $(TARGET)/parse.tab.c
	$(CC) $(CC-FLAGS) -c $(TARGET)/parse.tab.c -o $(TARGET)/parse.tab.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/librust.a: $(SRC)/lib.rs Cargo.toml $(SRC)/wrapper.h $(SRC)/assembly.rs $(SRC)/binary.rs $(SRC)/code.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) librust.a\n"
	$(RS) build $(RS-FLAGS)
	mv $(RS-TARGET)/librust.a $(TARGET)/librust.a
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"


# Gera a FFI entre C e Rust
.PHONY: bindings
bindings:
	@printf "$(BOLD)$(GREEN)Gerando bindings:$(NC)$(NORMAL)\n"
	cbindgen --config cbindgen.toml --output $(SRC)/rust.h
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"


# Limpa a saída
.PHONY: clean
clean:
	@printf "$(BOLD)$(GREEN)Removendo arquivos:$(NC)$(NORMAL)\n"
	-rm -rf $(TARGET)/*
	-rm -f $(BIN)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"
