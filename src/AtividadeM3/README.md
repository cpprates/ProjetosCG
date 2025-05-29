# ATIVIDADE MÓDULO 3
Projeto da Atividade do Módulo 3 da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

## Estrutura
O código da Vivencial1 foi utilizado como base e alterado para aceitar textura.

Foi utilizado os arquivos `Suzanne.obj` e `Cube.obj` da pasta `assets/Modelos3D` com a textura `pixelWall.png` da pasta `assets/tex`.

Foi alterado o CMakeLists.txt com o novo arquivo `AtividadeM3.cpp`

Foi adicionado as informações de textura em `vertexShaderSource` e `fragmentShaderSource` e no `loadSimpleOBJ`. Além disso, foi criado um novo método para carregar a textura `loadTexture()` a partir de um path de arquivo.

No `main()` foi adicionado o método da textura com `glUseProgram` e `glUniform1i`.

No loop também foi acionado a textura e o bind dela, respectivamente.

### Objetivo

O objetivo de adicionar textura aos objetos foi atingido, porém, a tarefa pede para que o arquivo lido seja .mtl. O trabalho ainda está em andamento, porém já foi possível visualizar a textura nos objetos carregados, o que foi bem divertido!

(sessão em andamento...)

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/AtividadeM3.cpp`

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./AtividadeM3
```

Ambiente utilizado: Linux

### Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: A, W, S, D, Q, E, Y, Z, X

Comandos do teclado para alterar objetos: `tab`

## Print do Programa em Execução

### Primeira versão da textura

![Texturizado v1](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM3/images/AtividadeM3-v1.png?raw=true)

### Segunda versão da textura

(em andamento)