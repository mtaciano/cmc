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
BIN-FLAGS = -static # Pode dar erro, remover se der

# Componentes do compilador
OBJS = $(TARGET)/parse.tab.o $(TARGET)/scan.yy.o $(TARGET)/main.o $(TARGET)/util.o $(TARGET)/symtab.o $(TARGET)/analyze.o $(TARGET)/intermediate.o
LIBS = $(TARGET)/libc.a $(TARGET)/librust.a


# Otimização de velocidade e tamanho
.PHONY: release
release: CC-FLAGS += -O3
release: RS-FLAGS += --release
release: RS-TARGET = $(TARGET)/release
release: $(BIN)
release:
	@printf "\n$(BOLD)$(GREEN)Removendo símbolos:$(NC)$(NORMAL) $(BIN)\n"
	strip -p --strip-all $(TARGET)/$(BIN)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Símbolos de debug
.PHONY: debug
debug: CC-FLAGS += -g -fsanitize=address,undefined -fno-omit-frame-pointer
debug: BISON-FLAGS += -g
debug: BIN-FLAGS = #empty
debug: RS-TARGET = $(TARGET)/debug
debug: $(BIN)


# Executável
$(BIN): $(OBJS) $(LIBS)
	@printf "$(BOLD)$(GREEN)Compilando:$(NC)$(NORMAL) $(BIN)\n"
	$(CC) $(LIBS) $(CC-FLAGS) $(BIN-FLAGS) -o $(TARGET)/$(BIN)
	ln -sf $(TARGET)/$(BIN) $(BIN)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Objetos
$(TARGET)/main.o: $(SRC)/main.c $(COMMON)/globals.h $(COMMON)/util.h $(SRC)/scan.h $(SRC)/analyze.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) main.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/main.c -o $(TARGET)/main.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/util.o: $(COMMON)/util.c $(COMMON)/util.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) util.o\n"
	$(CC) $(CC-FLAGS) -c $(COMMON)/util.c -o $(TARGET)/util.o
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/intermediate.o: $(SRC)/intermediate.c $(SRC)/intermediate.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL) intermediate.o\n"
	$(CC) $(CC-FLAGS) -c $(SRC)/intermediate.c -o $(TARGET)/intermediate.o
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

$(TARGET)/librust.a: $(SRC)/rust.rs Cargo.toml $(SRC)/wrapper.h $(SRC)/assembly.rs $(SRC)/binary.rs $(SRC)/intermediate.h $(COMMON)/globals.h
	@printf "$(BOLD)$(GREEN)Criando biblioteca:$(NC)$(NORMAL) librust.a\n"
	$(RS) build $(RS-FLAGS)
	mv $(RS-TARGET)/librust.a $(TARGET)/librust.a
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/libc.a: $(OBJS)
	@printf "$(BOLD)$(GREEN)Criando biblioteca:$(NC)$(NORMAL) libc.a\n"
	ar rcs $(TARGET)/libc.a $(OBJS)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"


# Copia o binário de saída para o processador
# Apenas usar caso tenha o processador na mesma pasta
.PHONY: cpu
cpu:
	@printf "$(BOLD)$(GREEN)Copiando binário:$(NC)$(NORMAL) output.txt\n"
	cp $(TARGET)/output.txt ../fpgmips/output.txt
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Limpa a saída
.PHONY: clean
clean:
	@printf "$(BOLD)$(GREEN)Removendo arquivos:$(NC)$(NORMAL)\n"
	-rm -rf $(TARGET)/*
	-rm -f $(BIN)
	@printf "$(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Para compilar na UNIFESP:
# make CC=gcc CC-FLAGS="-Wall -Wextra -pedantic -ldl -pthread" BIN-FLAGS= release
# make CC=gcc CC-FLAGS="-Wall -Wextra -pedantic -ldl -pthread -g -fsanitize=address,undefined -fno-omit-frame-pointer" debug
