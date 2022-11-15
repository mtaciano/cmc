# Compilador para C-

## Sobre

Nesse repositório está o compilador para um _subset_ da linguagem C, conhecido como C-.

Todas as etapas foram implementadas, sendo elas:

- [x] Lexer
- [x] Parser
- [x] Analyser
- [x] Gerador de código intermediário
- [x] Gerador de cõdigo assembly
- [x] Gerador de cõdigo binário

Pode-se melhorar diversas partes do código. No entanto, para o propósito desejado, a implementação atual é o suficiente.

## Instalação
Após clonar o repositório, entre nele e crie a pasta para onde os arquivos de compilação serão armazenados:
```
cd cmc
mkdir build
```

Para realizar o desenvolvimento do compilador `cmc` é recomendado usar seguinte comando:
```
make debug # Gera código ideal para o desenvolvimento
```

Para utilizar o compilador `cmc` como um produto final é recomendado usar o seguinte comando:
```
make release # Gera código otimizado em tanto tamanho e velocidade
```

## Dependências
- Rust
- Clang
- Bison
- Flex

## Contribuição

Caso ache erros, use a aba de _issues_ para relatá-los.

## Licença

A licença usada para o compilador é a licença permissiva MIT.
