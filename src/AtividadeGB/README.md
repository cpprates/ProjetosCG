# Integralização GB | Cena Final — OVNI vs Vaca 🛸🐄

Este projeto é uma cena interativa em OpenGL, desenvolvida com C++, onde um OVNI realiza a abdução de uma vaca em um cenário com casa, chão texturizado, skybox e movimentação de câmera. O projeto ilustra diversos conceitos de computação gráfica, como iluminação, carregamento de modelos 3D com materiais, animações, uso de shaders e interação via teclado e mouse.

Alunos: Carolina Paz Prates, Kevin Mulinari Kuhn, Vitor Mello

## 📜 Organização do Projeto

```
.
├── 📁 src/CenaFinal.cpp                // Código-fonte da Cena
├── 📁 src/AtividadeGB/                 // README.md & imagens & documentos
├── 📁 src/assets/Modelos3D/final/      // Objetos utilizados

```

## 🎬 Descrição Geral da Cena

- Um **OVNI animado** sobrevoa a cena e interage com uma **vaca animada**.
- A vaca pode ser **abduzida ou solta**, realizando um movimento suave com curvas.
- Uma **casa** ilumina a vaca enquanto o OVNI está espionando.
- O ambiente é envolto por uma **skybox** (plano de fundo animado).
- O usuário pode **navegar pela cena com mouse e teclado**, como em jogos em primeira pessoa.

## 🔧 Organização do Código

### 🔹 Shaders

O projeto utiliza um **shader program principal** (`shaderProgram`) e um **shader separado para o skybox** (`skyboxShader`). Os shaders controlam a aparência visual dos objetos com base na iluminação ambiente, difusa e especular, além de aplicar as texturas.

### 🔹 Arquivo de Configuração (`config.ini`)

Parâmetros como posições de luz, materiais dos objetos e caminhos dos modelos e texturas são definidos externamente em um arquivo `config.ini`. Isso permite personalizar a cena sem recompilar o código.

Funções utilitárias para acessar esses dados:

- `getFloat("chave", valorPadrao)`  
- `getVec3("chave", vec3Padrao)`  
- `getString("chave", valorPadrao)`

Esses métodos acessam um `map<string, string>` populado no início da execução com `loadConfig("config.ini")`.

### Parser de arquivo `.ini`

Inspirado neste tutorial: [*“A simple C++ INI file reader”*](https://tuttlem.github.io/2012/12/26/a-simple-c-ini-file-reader.html) — fornece um parser leve que:

- Divide o arquivo em **seções** (`[window]`, `[modelo_paths]`, etc.)
- Armazena cada `chave=valor` em um `std::map<string, map<string,string>>`
- Permite recuperar os valores com chave composta: `config["window"]["title"]`, etc.

O código do tutorial mostra exatamente a lógica usada no nosso `loadConfig`.
O projeto carrega parâmetros configuráveis de um arquivo `config.ini` com base em seções e chaves. O carregamento é feito pelo parser `.ini` próprio escrito em C++.

---

## 📦 Carregamento de Modelos (`loadModel` e `loadOBJWithMTL`)

### Modelo com Submalhas

Cada modelo (`Modelo`) possui múltiplas **submalhas (`Submesh`)**, permitindo múltiplos materiais e texturas no mesmo `.obj`.

```cpp
struct Submesh {
    vector<Vertex> vertices;
    GLuint VAO, VBO, textureID;
    int vertexCount;
    Material material;
};
```

O método `loadModel()` usa `loadOBJWithMTL()` para carregar:

- Vértices, normais e coordenadas UV.
- Materiais definidos no arquivo `.mtl`.
- Texturas correspondentes.
- Para cada grupo de material (`usemtl`), cria-se um `Submesh`.

Isso garante que diferentes partes da nave, vaca ou casa possam ter materiais distintos com fidelidade visual ao que foi exportado do Blender.

## 🎮 Interatividade e Animações

### 🧲 Animação da Abdução

Quando a luz da casa se apaga (`casaLuz == false`), o OVNI **desce** e a vaca é **abduzida com uma curva suave**:

```cpp
vacaX = curvaAmplitude * sin(t * 2.0f);         // Movimento em X
vacaZ = curvaAmplitude * sin(t * 2.0f) * cos(t * 2.0f); // Movimento em Z
```

Esse movimento simula uma trajetória de espiral ou oscilatória, dando vida à animação da vaca sendo sugada pela nave.

Quando a luz da casa acende novamente, o OVNI sobe e a vaca desce com rotação, até tocar o chão, resetando sua rotação.

### 🧭 Câmera e Navegação

- A câmera é uma `FPS Camera` com movimentação em primeira pessoa.
- Usa o teclado (`WASD`, etc.) para movimentação.
- Usa o mouse para rotação.
- O cursor é desabilitado com `glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED)` e controlado por `glfwSetCursorPosCallback(w, mouse_callback)`.

## 🧪 Iluminação

O sistema de iluminação da cena utiliza o modelo **Phong**, que combina três componentes:
- **Ambiental (ka)**: simula luz indireta constante.
- **Difusa (kd)**: depende da direção da luz em relação à normal da superfície.
- **Especular (ks)**: simula o brilho causado pela reflexão da luz, afetado pelo parâmetro `shininess`.

Esses valores são passados ao shader principal e controlam como cada objeto interage com a luz na cena.

Além disso, há dois cenários alternando com base na variável `casaLuz`:

1. **Luz da Casa**:
   - Luz branca posicionada na casa, com foco direcionado à vaca.
   - Direção é calculada dinamicamente entre o telhado e a posição da vaca.
   - Tons quentes.
   - A iluminação é modelada como um *spotlight* com ângulo de corte (cone de luz) definido no fragment shader via `spotCutOff`.

2. **Luz do OVNI**:
   - Luz esverdeada posicionada abaixo do OVNI.
   - Direção fixa para baixo, simulando o feixe de abdução.
   - Acompanha o movimento vertical do OVNI.
   - Também implementa iluminação com cone de luz, utilizando `spotCutOff` para delimitar a área iluminada, reforçando o efeito de feixe.

---

## 📈 Possíveis Melhorias

Aqui estão algumas sugestões de evolução para o projeto:

- **Controlar o OVNI com o jogador** (teclado/mouse) e permitir voar pela cena.
- **Adicionar mais animais** com diferentes reações (correndo, fugindo, sendo abduzidos).
- **Sistema de partículas** para feixe de abdução.

---

## 🛠 Tecnologias Utilizadas

- C++17
- OpenGL 3.3+
- GLFW
- GLAD
- GLM
- stb_image
- Blender (modelagem e exportação OBJ/MTL)

## 📚 Referências

[INI file](https://en.wikipedia.org/wiki/INI_file)
[Learn OpenGL Tutoriais](https://learnopengl.com/Code-repository)
[OpenGL objetos e texturas](https://github.com/srcres258/learnopengl-rust/tree/master)
[Github Professora Rossana](https://github.com/fellowsheep/CG2024-2/)

---

## 👩‍💻 Autoria

Projeto desenvolvido por [@cpprates](https://github.com/cpprates), [@kevin-kuhn](https://github.com/kevin-kuhn) e [Vitor Mello]() como prática de conceitos de computação gráfica com foco em animações, modelos 3D e shaders.

---

