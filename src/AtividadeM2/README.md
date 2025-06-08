# ATIVIDADE MÓDULO 2
Projeto da Atividade do Módulo 2 da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

## Estrutura
O código `Hello3D.cpp` foi utilizado como base e alterado conforme pedido na tarefa.
Foi alterado o arquivo `CMakeLists.txt` com o novo arquivo `AtividadeM2.cpp`

### Geometria com Cores

No método `setupGeometry`, foram adicionados **12 triângulos com cores distintas**, resultando em um cubo colorido. Cada face do cubo foi representada com duas metades triangulares para possibilitar a aplicação de cores únicas por triângulo.

### Translação com Teclas WASD + IJ
No método `key_callback`, foi adicionada a movimentação do cubo nos eixos x, y, z por meio das teclas:
- `A` e `D`: movimentação no eixo X (esquerda/direita)
- `W` e `S`: movimentação no eixo Z (profundidade frente/trás)
- `I` e `J`: movimentação no eixo Y (cima/baixo)

### Profundidade com View e Projection
Para permitir uma percepção realista de profundidade na cena:
- O `vertexShaderSource` foi atualizado para incluir as matrizes `view` e `projection`.
- No `main`, foi definida uma `view matrix` para simular a posição da câmera e uma `projection matrix` com perspectiva 3D realista, utilizando `glm::perspective`.

### Escala Uniforme com Teclas `[` e `]`
Foi implementada uma funcionalidade de **escala uniforme** utilizando os colchetes `[` e `]` para aumentar ou diminuir o tamanho do cubo. Para isso:
- Criou-se um novo método `character_callback`, que utiliza `codepoint` para capturar corretamente os caracteres `[` e `]`, pois o `key_callback` padrão não reconhece esses símbolos corretamente em alguns layouts de teclado.

### Instanciando Múltiplos Cubos
O loop principal (`main`) foi modificado para instanciar **dois cubos na mesma cena**. Ambos compartilham uma **matriz base comum** para realizar os mesmos movimentos (translação, rotação e escala), mas com posições distintas na tela. Isso permite observar a transformação simultânea em objetos diferentes, mantendo a lógica simples e elegante.

#### Objetivo

O objetivo de alterar a geometria da pirâmide para transformar em um cubo foi atingido, assim como aplicação da translação e da escala do cubo além de instanciar mais de um cubo na cena.

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/AtividadeM2.cpp`

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./AtividadeM2
```

> Ambiente utilizado: Linux

### Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: `A`, `W`, `S`, `D`, `I`, `J`

Comandos do teclado para promover escala uniforme dos cubos: `[`, `[` 

## Print do Programa em Execução

![Cubos1](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM2/images/AtividadeM2-1.png?raw=true)

![Cubos2](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM2/images/AtividadeM2-2.png?raw=true)

![Cubos3](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM2/images/AtividadeM2-3.png?raw=true)