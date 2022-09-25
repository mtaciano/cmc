# Compilador para C-

## Sobre

Dentro deste repositório é possível achar todas etapas de um compilador para um _subset_ da linguagem C, conhecido como C-.

As etapas atualmente implementadas são:

- [x] Lexer
- [x] Parser
- [x] Analyser
- [x] Gerador de código intermediário (Alguns bugs existem)

Etapas parcialmente implementadas:

- [x] Gerador de código assembly (Alguns bugs existem)
- [x] Gerador de código de máquina (processador de arquitetura própria)

Etapas faltando:
XXX

## Instalação

```
make debug # Gera código para desenvolvimento
```
```
make release # Gera código otimizado
```

## Dependências
- Cargo (rustup)
- Clang
- Bison
- Flex

## Contribuição

Atualmente o repositório não está aberto para contribuições.

## Licença

Atualmente não há licença para o projeto.
