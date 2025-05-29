#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assert.h>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

struct Object3D {
    GLuint vao;
    int vertexCount;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
};

const GLuint WIDTH = 1000, HEIGHT = 1000;
std::vector<Object3D> objects;
int selected = 0;

extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = aColor;
    TexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";

GLuint setupShader();

int loadSimpleOBJ(string filePath, int &nVertices)
{
    vector<glm::vec3> vertices;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normals;
    vector<GLfloat> vBuffer;

    ifstream arqEntrada(filePath);
    if (!arqEntrada.is_open()) {
        cout << "Erro ao tentar ler o arquivo " << filePath << endl;
        return -1;
    }

    string line;
    while (getline(arqEntrada, line)) {
        istringstream ssline(line);
        string word;
        ssline >> word;
        if (word == "v") {
            glm::vec3 vert;
            ssline >> vert.x >> vert.y >> vert.z;
            vertices.push_back(vert);
        } else if (word == "vt") {
            glm::vec2 vt;
            ssline >> vt.x >> vt.y;
            texCoords.push_back(vt);
        } else if (word == "vn") {
            glm::vec3 norm;
            ssline >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        } else if (word == "f") {
            while (ssline >> word) {
                int vi, ti, ni;
                sscanf(word.c_str(), "%d/%d/%d", &vi, &ti, &ni);
                vi--; ti--; ni--;

                glm::vec3 v = vertices[vi];
                glm::vec2 t = texCoords[ti];

                vBuffer.push_back(v.x);
                vBuffer.push_back(v.y);
                vBuffer.push_back(v.z);

                // cor qualquer para compatibilidade com layout antigo
                vBuffer.push_back(1.0f);
                vBuffer.push_back(1.0f);
                vBuffer.push_back(1.0f);

                vBuffer.push_back(t.x);
                vBuffer.push_back(t.y);
            }
        }
    }

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = vBuffer.size() / 8;
    return VAO;
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Wrapping / Filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CG - Carolina Prates | Atividade M3", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = setupShader();
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	GLuint texture = loadTexture("../assets/tex/pixelWall.png");
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    // Carregar objetos
    vector<string> paths = {        
        "../assets/Modelos3D/Cube.obj",
	    "../assets/Modelos3D/Suzanne.obj"
    };
    for (const auto& path : paths) {
        Object3D obj;
        obj.vao = loadSimpleOBJ(path, obj.vertexCount);
        objects.push_back(obj);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.9, 0.9, 0.9, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (int i = 0; i < objects.size(); ++i) {
            Object3D& obj = objects[i];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::rotate(model, obj.rotation.x, glm::vec3(1, 0, 0));
            model = glm::rotate(model, obj.rotation.y, glm::vec3(0, 1, 0));
            model = glm::rotate(model, obj.rotation.z, glm::vec3(0, 0, 1));
            model = glm::scale(model, obj.scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(obj.vao);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    Object3D& obj = objects[selected];

    switch (key) {
        case GLFW_KEY_TAB: selected = (selected + 1) % objects.size(); break;
        case GLFW_KEY_X: obj.rotation.x += glm::radians(10.0f); break;
        case GLFW_KEY_Y: obj.rotation.y += glm::radians(10.0f); break;
        case GLFW_KEY_Z: obj.rotation.z += glm::radians(10.0f); break;
        case GLFW_KEY_W: obj.position.z -= 0.1f; break;
        case GLFW_KEY_S: obj.position.z += 0.1f; break;
        case GLFW_KEY_A: obj.position.x -= 0.1f; break;
        case GLFW_KEY_D: obj.position.x += 0.1f; break;
        case GLFW_KEY_Q: obj.position.y += 0.1f; break;
        case GLFW_KEY_E: obj.position.y -= 0.1f; break;
        case GLFW_KEY_KP_ADD: obj.scale *= 1.1f; break;
        case GLFW_KEY_KP_SUBTRACT: obj.scale *= 0.9f; break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
    }
}

GLuint setupShader() {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return shaderProgram;
}
