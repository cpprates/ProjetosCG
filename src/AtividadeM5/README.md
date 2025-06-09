# ATIVIDADE MÓDULO 5
Projeto da Atividade do Módulo 5 da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

## Estrutura
O código da Atividade M4 foi utilizado como base e alterado para aceitar câmera.

Foi utilizado o arquivo `Suzanne.obj` da pasta `assets/Modelos3D`.

Foi alterado o CMakeLists.txt com o novo arquivo `AtividadeM5.cpp` e também para reconhecer `include/Camera.h` e `common/Camera.cpp`

### Câmera

A movimentação da câmera é feita com base nos vetores:

- `cameraPos`: posição atual da câmera no mundo.
- `cameraFront`: direção para onde a câmera está olhando.
- `cameraUp`: vetor que representa o "topo" da câmera.

O `view matrix` é criado com `glm::lookAt`, utilizando a fórmula:
```cpp
glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```
Com isso, a câmera sempre olha na direção determinada por `cameraFront`, e a orientação vertical é dada por `cameraUp`.

A movimentação em 4 direções (frente, trás, esquerda, direita) altera `cameraPos` com base nesses vetores.

### Movimentações e Callback

#### Teclado

As teclas `W`, `S`, `A`, `D` controlam a movimentação para frente, trás, esquerda e direita, respectivamente.

O método `ProcessKeyboard()` da classe `Camera` calcula a nova posição com base no vetor `Front` (direção para frente) e `Right` (direita calculada com produto vetorial entre `Front` e `Up`).

#### Mouse

O movimento do mouse altera o ângulo da câmera. O callback `mouse_callback` calcula o deslocamento do mouse e chama:

```cpp
camera.ProcessMouseMovement(xoffset, yoffset);
```

Isso altera os ângulos de Euler (`Yaw` e `Pitch`) e atualiza o vetor `Front`, mudando a direção da câmera.

#### Scroll (Zoom)

A rolagem do mouse altera o campo de visão (zoom), limitado entre 1.0 e 45.0 graus. O callback `scroll_callback` chama:

```cpp
camera.ProcessMouseScroll(yoffset);
```

---

### Direção e Ângulos de Euler

A direção da câmera é calculada usando **ângulos de Euler**, que permitem representar rotações em 3D de maneira intuitiva.

A fórmula usada é:

```cpp
front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
front.y = sin(glm::radians(Pitch));
front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
Front = glm::normalize(front);
```

Essa abordagem gera um vetor de direção baseado nos ângulos horizontais (`Yaw`) e verticais (`Pitch`), simulando um sistema de câmera FPS.

substitui a primeira tentativa do material que foi e fazia a rotação do objeto também:
```cpp
front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
front.y = sin(glm::radians(pitch));
front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
```
### Extração da Câmera (Modularização)

Foi feita a **refatoração do código**, separando toda a lógica da câmera em uma classe `Camera`, dividida em:

- `Camera.h` – Declaração da classe e seus métodos públicos.
- `Camera.cpp` – Implementação dos métodos, como movimentação com teclado, rotação com mouse e zoom.

No código principal:

- Incluído `#include "Camera.h"`
- Criada uma instância global da câmera para uso nas callbacks.
- Substituídas as variáveis globais como `cameraPos`, `cameraFront`, etc., por `camera.Position`, `camera.Front`, etc.

## Objetivo

O objetivo de implementar a câmera seguindo o material disponibilizado e também o desafio de modularizar a Câmera foi atingido.

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/AtividadeM5.cpp`

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./AtividadeM5
```

> Ambiente utilizado: Linux

## Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: `A`, `W`, `S`, `D`

Comandos do mouse: movimento da câmera e scroll para zoom

## Print do Programa em Execução

![Camera1](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM5/images/AtividadeM5-1.png?raw=true)

![Camera2](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM5/images/AtividadeM5-2.png?raw=true)