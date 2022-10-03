# Compilador para C-

## Sobre

Nesse repositõrio está o compilador para um _subset_ da linguagem C, conhecido como C-.

As etapas atualmente implementadas são:

- [x] Lexer
- [x] Parser
- [x] Analyser
- [x] Gerador de código intermediário (Alguns bugs existem)
- [x] Gerador de cõdigo assembly (Alguns bugs existem)
- [x] Gerador de cõdigo binãrio (Alguns bugs existem)

## Instalação

Para realizar o desenvolvimento do `cmc` é recomendado usar seguinte comando:
```
make debug # Gera código para desenvolvimento
```

Enquanto que para usar o compilador é recomendado usar o próximo comando:
```
make release # Gera código otimizado em tanto tamanho e velocidade
```

## Dependências
- Cargo (rustup)
- Clang
- Bison
- Flex

## Contribuição

Caso ache bugs, use a aba de _issues_ para relatá-los.

## Licença

A licença usada para o compilador é a licença permissiva MIT. 
