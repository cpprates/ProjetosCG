# ATIVIDADE VIVENCIAL 2 - 31/05/2025
Projeto da Atividade Vivencial 2 da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

Kevin Mulinari Kuhn

Vitor Mello

## Objetivo

Simular uma cena 3D interativa com um objeto iluminado por três fontes de luz distintas, seguindo o esquema clássico de iluminação cinematográfica:

* Key Light: a principal fonte de luz, mais intensa.
* Fill Light: luz de preenchimento, suaviza sombras.
* Back Light: luz de contraluz, destaca o contorno do objeto.

As luzes podem ser ativadas ou desativadas dinamicamente durante a execução, pressionando as teclas `1`, `2` e `3` no teclado:

* Tecla `1` alterna a **Key Light**
* Tecla `2` alterna a **Fill Light**
* Tecla `3` alterna a **Back Light**

### Estrutura

Utilizamos o arquivo `Cube.obj` da pasta `assets/Modelos3D`

Alteramos CMakeLists.txt com o novo arquivo `cpp`

Foram adicionadas três fontes de luz posicionadas em relação ao objeto:

Key Light (`objectPos + vec3(2.0f, 2.0f, 2.0f)`), Fill Light (`objectPos + vec3(-2.0f, 1.0f, 1.5f)`), Back Light (`objectPos + vec3(0.0f, 2.0f, -2.5f)`)

A iluminação é computada no Fragment Shader, com suporte a:

* Componentes **ambiental**, **difusa** e **especular** e **atenuação** baseada na distância.
* Controle dinâmico das luzes com variáveis booleanas (`lightOn[i]`)

Além disso, implementamos em cima do trabalho de câmera que implementa movimentos com teclado e rotação com o mouse.

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/Vivencial2.cpp`

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./Vivencial2
```
> Ambiente: Linux 🐧

### Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: `A`, `W`, `S`, `D`

Comandos do teclado para alterar luzes: `1`, `2`, `3`

Comandos do mouse alteram a câmera em relação ao objeto

## Prints do Programa em Execução

![Cube1](https://github.com/cpprates/ProjetosCG/blob/main/src/Vivencial2/images/Vivencial2-keyON.png?raw=true)

![Cube2](https://github.com/cpprates/ProjetosCG/blob/main/src/Vivencial2/images/Vivencial2-keyOFF.png?raw=true)

![Cube3](https://github.com/cpprates/ProjetosCG/blob/main/src/Vivencial2/images/Vivencial2-backON.png?raw=true)

![Cube4](https://github.com/cpprates/ProjetosCG/blob/main/src/Vivencial2/images/Vivencial2-backOFF.png?raw=true)