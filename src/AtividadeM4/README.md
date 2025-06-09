# ATIVIDADE MÓDULO 4
Projeto da Atividade do Módulo 4 da Disciplina de Computação Gráfica da Unisinos Porto Alegre (Híbrido)

## Alunos 
Carolina Paz Prates

## Estrutura
O código da Atividade M3 foi utilizado como base e alterado para aceitar iluminação.

Foi utilizado o arquivo `Suzanne.obj` da pasta `assets/Modelos3D`.

Foi alterado o CMakeLists.txt com o novo arquivo `AtividadeM4.cpp`

### Vetores Normais dos Vértices

- No método `loadSimpleOBJ()` foram adicionadas as leituras das propriedades do material presentes no arquivo `.mtl`, incluindo:
  - `Ka` (coeficiente ambiente)
  - `Kd` (coeficiente difuso)
  - `Ks` (coeficiente especular)
  - `Ns` (expoente de brilho)
- As informações de normais dos vértices também foram adicionadas.
- O tamanho da estrutura de dados dos vértices foi alterado de 8 para 11 atributos para acomodar:
  - Posição (x, y, z)
  - Cor (r, g, b)
  - Coordenadas de textura (u, v)
  - Vetor normal (nx, ny, nz)

### Coeficiente de Iluminação

- No `vertexShaderSource` e `fragmentShaderSource` foram adicionadas as implementações para calcular os três componentes do modelo de iluminação de Phong:
  - **Iluminação Ambiente**: `ambient = light.ambient * material.ambient;`
  - **Iluminação Difusa**: `diffuse = light.diffuse * (diff * material.diffuse);`
  - **Iluminação Especular**: `specular = light.specular * (spec * material.specular);`
- O vetor normal foi transformado para o espaço do mundo com a expressão:
  ```glsl
  Normal = mat3(transpose(inverse(model))) * aNormal;
  ```

### Instanciação de Luz e Material no `main()`

- Foram instanciadas e passadas para o shader:
  - As propriedades da **luz** (`light`) contendo:
    - `position`
    - `ambient`
    - `diffuse`
    - `specular`
  - As propriedades do **material** (`material`) contendo:
    - `ambient`
    - `diffuse`
    - `specular`
    - `shininess` (`Ns`)
- Esses dados são enviados aos shaders por meio de `glUniform`.


### Objetivo

O objetivo de adicionar iluminação de Modelo Phong foi atingido com sucesso, juntamente com a leitura dos vetores normais dos vértices no arquivo .OBJ.

## Como rodar

O arquivo `cpp` dessa atividade está localizado em `src/AtividadeM4.cpp`

```
mkdir build (caso não exista a pasta build/)
cd build/
cmake .. 
make 
./AtividadeM4
```

> Ambiente utilizado: Linux

## Execução do Programa

Comandos do teclado para transladar e movimentar os objetos: `A`, `W`, `S`, `D`, `Y`, `Z`, `X`, `I`, `J`

### Print do Programa em Execução

![Phong1](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM4/images/AtividadeM4-1.png?raw=true)

![Phong2](https://github.com/cpprates/ProjetosCG/blob/main/src/AtividadeM4/images/AtividadeM4-2.png?raw=true)