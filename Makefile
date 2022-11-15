# Makefile para o compilador C-

# Cores
BOLD := $(shell tput bold)
NORMAL := $(shell tput sgr0)
GREEN := $(shell echo "\033[0;32m")
NC := $(shell echo "\033[0m") # No color

# Pastas
SRC = src
COMMON = src/common
TARGET = build
RSTARGET = #empty

# C/C++
CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Bison e Flex
BISON = bison
BFLAGS = -W --color=always -d
LEX = flex

# Rust
RS = cargo
RSFLAGS = #empty

# C Minus Compiler
BIN = cmc
BINFLAGS = -static # Pode dar erro, remover se der
LDFLAGS = -fuse-ld=mold -Wl,-O1 -Wl,--as-needed # Se der erro, remover `-fuse-ld=mold` pois não está no sistema

# Componentes do compilador
OBJS = $(TARGET)/parse.tab.o $(TARGET)/scan.yy.o $(TARGET)/main.o $(TARGET)/util.o $(TARGET)/symtab.o $(TARGET)/analyze.o $(TARGET)/intermediate.o
LIBS = $(TARGET)/libc.a $(TARGET)/librust.a


# Otimização de velocidade e tamanho
.PHONY: release
release: CFLAGS += -march=native -flto -O3
release: RSFLAGS += --release
release: RSTARGET = $(TARGET)/release
release: $(BIN)
release:
	@printf "\n==> $(BOLD)$(GREEN)Removendo símbolos:$(NC)$(NORMAL)$(BIN)\n"
	strip -p --strip-all $(TARGET)/$(BIN)
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Símbolos de debug
.PHONY: debug
debug: CFLAGS += -g -fsanitize=address,undefined -fno-omit-frame-pointer
debug: BFLAGS += -g
debug: BINFLAGS = #empty
debug: RSTARGET = $(TARGET)/debug
debug: $(BIN)


# Executável
$(BIN): $(OBJS) $(LIBS)
	@printf "==> $(BOLD)$(GREEN)Compilando:$(NC)$(NORMAL)$(BIN)\n"
	$(CC) $(LIBS) $(CFLAGS) $(BINFLAGS) $(LDFLAGS) -o $(TARGET)/$(BIN)
	ln -sf $(TARGET)/$(BIN) $(BIN)
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Objetos
$(TARGET)/main.o: $(SRC)/main.c $(COMMON)/globals.h $(COMMON)/util.h $(SRC)/scan.h $(SRC)/analyze.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)main.o\n"
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $(TARGET)/main.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/util.o: $(COMMON)/util.c $(COMMON)/util.h $(COMMON)/globals.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)util.o\n"
	$(CC) $(CFLAGS) -c $(COMMON)/util.c -o $(TARGET)/util.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/intermediate.o: $(SRC)/intermediate.c $(SRC)/intermediate.h $(COMMON)/globals.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)intermediate.o\n"
	$(CC) $(CFLAGS) -c $(SRC)/intermediate.c -o $(TARGET)/intermediate.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/symtab.o: $(SRC)/symtab.c $(SRC)/symtab.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)symtab.o\n"
	$(CC) $(CFLAGS) -c $(SRC)/symtab.c -o $(TARGET)/symtab.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/analyze.o: $(SRC)/analyze.c $(COMMON)/globals.h $(SRC)/symtab.h $(SRC)/analyze.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)analyze.o\n"
	$(CC) $(CFLAGS) -c $(SRC)/analyze.c -o $(TARGET)/analyze.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/scan.yy.o: $(SRC)/scan.l $(SRC)/scan.h $(COMMON)/util.h $(COMMON)/globals.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)scan.yy.o\n"
	$(LEX) -o $(TARGET)/scan.yy.c $(SRC)/scan.l
	$(CC) $(CFLAGS) -c $(TARGET)/scan.yy.c -o $(TARGET)/scan.yy.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/parse.tab.o: $(SRC)/parse.y $(COMMON)/globals.h
	@printf "==> $(BOLD)$(GREEN)Criando objeto:$(NC)$(NORMAL)parse.tab.o\n"
	$(BISON) $(BFLAGS) $(SRC)/parse.y -o $(TARGET)/parse.tab.c
	$(CC) $(CFLAGS) -c $(TARGET)/parse.tab.c -o $(TARGET)/parse.tab.o
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/librust.a: $(SRC)/rust.rs Cargo.toml $(SRC)/wrapper.h $(SRC)/assembly.rs $(SRC)/binary.rs $(SRC)/intermediate.h $(COMMON)/globals.h
	@printf "==> $(BOLD)$(GREEN)Criando biblioteca:$(NC)$(NORMAL)librust.a\n"
	$(RS) build $(RSFLAGS)
	mv -f $(RSTARGET)/librust.a $(TARGET)/librust.a
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"

$(TARGET)/libc.a: $(OBJS)
	@printf "==> $(BOLD)$(GREEN)Criando biblioteca:$(NC)$(NORMAL)libc.a\n"
	$(AR) rcs $(TARGET)/libc.a $(OBJS)
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n\n"


# Copia o binário de saída para o processador
# Apenas usar caso tenha o processador na mesma pasta
.PHONY: cpu
cpu:
	@printf "==> $(BOLD)$(GREEN)Copiando binário:$(NC)$(NORMAL)output.txt\n"
	cp $(TARGET)/output.txt ../fpgmips/output.txt
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Limpa a saída
.PHONY: clean
clean:
	@printf "==> $(BOLD)$(GREEN)Removendo arquivos:$(NC)$(NORMAL)\n"
	$(RM) -rv $(TARGET)
	$(RM) $(BIN)
	mkdir $(TARGET)
	@printf "==> $(BOLD)$(GREEN)sucesso!$(NC)$(NORMAL)\n"


# Para compilar na UNIFESP:
# make CC=gcc CFLAGS="-Wall -Wextra -pedantic -ldl -pthread" BINFLAGS= release
# make CC=gcc CFLAGS="-Wall -Wextra -pedantic -ldl -pthread -g -fsanitize=address,undefined -fno-omit-frame-pointer" debug
