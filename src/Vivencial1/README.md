# ATIVIDADE VIVENCIAL 1 - 17/05/2025
Projeto da Atividade Vivencial da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

Kevin Mulinari Kuhn

Vitor Mello

## Estrutura
Utilizamos os arquivos `Suzanne.obj` e `Cube.obj` da pasta `assets/Modelos3D`

Alteramos CMakeLists.txt com o novo arquivo `cpp`

Adicionamos os shaders e configuramos o arquivo para que fosse possível instanciar dois objetos diferentes lidos de arquivos .obj

### Objetivo

Conseguimos atingir o objetivo de entregar dois objetos lidos de arquivos .obj, rotacionar e movimentar conforme solicitado e trocar a seleção de cada objeto para realizar as rotações/alterações.

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/Vivencial1.cpp`

Observação: Utilizamos ambiente Linux para essa atividade

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./Vivencial1
```

### Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: A, W, S, D, Q, E, Y, Z, X

Comandos do teclado para alterar objetos: `tab`

## Print do Programa em Execução

![Suzanne & Cube](https://github.com/cpprates/ProjetosCG/blob/main/src/Vivencial1/images/Vivencial1.png?raw=true)