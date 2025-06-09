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

struct Material {
    glm::vec3 ambient{0.2f, 0.2f, 0.2f};
    glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess = 32.0f;
};

struct Object3D {
    GLuint vao;
    GLuint textureID;
    int vertexCount;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
	Material material;
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
layout(location = 3) in vec3 aNormal;

out vec3 vertexColor;
out vec3 Normal;
out vec2 TexCoord;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = aColor;
    TexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPos, 1.0)); // posição no mundo
    Normal = mat3(transpose(inverse(model))) * aNormal; // normal no mundo
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
in vec2 TexCoord;
in vec3 FragPos; // posição no mundo
in vec3 Normal;  

out vec4 FragColor;

uniform vec3 viewPos;

// Estrutura do material
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material material;

// Estrutura da luz
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

uniform sampler2D texture1;

void main()
{
    // Normalização da normal e direção da luz
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // Iluminação Ambiente
    vec3 ambient = light.ambient * material.ambient;

    // Iluminação Difusa
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // Iluminação Especular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // Cor final combinada
    vec3 result = ambient + diffuse + specular;

    // Multiplica pela textura
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = vec4(result, 1.0) * texColor;
}
)";

GLuint setupShader();

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

int loadSimpleOBJ(string filePath, int &nVertices, GLuint &textureID, Material &material)
{
    vector<glm::vec3> vertices;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normals;
    vector<GLfloat> vBuffer;
    string mtlFileName;
	string textureName;

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

        if (word == "mtllib")
        {
            // pega o nome do arquivo .mtl
            ssline >> mtlFileName;

            // le o arquivo .mtl para pegar a textura
            ifstream mtlFile("../assets/Modelos3D/" + mtlFileName);
            if (mtlFile.is_open())
            {
                string mtlLine;
                while (getline(mtlFile, mtlLine))
                {
                    istringstream ssMtl(mtlLine);
                    string mtlWord;
                    ssMtl >> mtlWord;

                    if (mtlWord == "map_Kd") {
						ssMtl >> textureName;
					} else if (mtlWord == "Ka") {
						ssMtl >> material.ambient.r >> material.ambient.g >> material.ambient.b;
					} else if (mtlWord == "Kd") {
						ssMtl >> material.diffuse.r >> material.diffuse.g >> material.diffuse.b;
					} else if (mtlWord == "Ks") {
						ssMtl >> material.specular.r >> material.specular.g >> material.specular.b;
					} else if (mtlWord == "Ns") {
						ssMtl >> material.shininess;
					}
                }
            }
        }

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
                t.y = 1.0f - t.y; // Invertendo Y para compatibilidade com OpenGL
				glm::vec3 n = normals[ni];

                vBuffer.push_back(v.x);
                vBuffer.push_back(v.y);
                vBuffer.push_back(v.z);

                // cor qualquer para compatibilidade com layout antigo
                vBuffer.push_back(1.0f);
                vBuffer.push_back(1.0f);
                vBuffer.push_back(1.0f);

                vBuffer.push_back(t.x);
                vBuffer.push_back(t.y);

				// normal
				vBuffer.push_back(n.x);
				vBuffer.push_back(n.y);
				vBuffer.push_back(n.z);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    textureID = 0;
    if (!textureName.empty())
    {
        textureID = loadTexture(("../assets/Modelos3D/" + textureName).c_str());
    }

    nVertices = vBuffer.size() / 11;
    return VAO;
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CG - Carolina Prates | Atividade M4", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = setupShader();
	glUseProgram(shaderProgram);

	// Definindo a luz
	glm::vec3 lightPos(2.0f, 2.0f, 2.0f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.position"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.ambient"), 1, glm::value_ptr(glm::vec3(0.2f)));
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.diffuse"), 1, glm::value_ptr(glm::vec3(0.7f)));
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.specular"), 1, glm::value_ptr(glm::vec3(1.0f)));
    
	// Posição da câmera (fixa para essa atividade)
	glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

	glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), 1.2f, 1.0f, 2.0f);

    // Carregar objetos
    vector<string> paths = {"../assets/Modelos3D/Suzanne.obj"};
    for (const auto& path : paths) {
        Object3D obj;
		Material material;
        GLuint tex;
        obj.vao = loadSimpleOBJ(path, obj.vertexCount, tex, material);
	    obj.textureID = tex;
		// posicoes iniciais
		obj.position = glm::vec3(0.0f, 0.0f, 0.0f);
		obj.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		obj.scale = glm::vec3(1.0f);

        objects.push_back(obj);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.9, 0.9, 0.9, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/HEIGHT, 0.1f, 100.0f);
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
            
			// Setar o material
			glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1, glm::value_ptr(obj.material.ambient));
			glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, glm::value_ptr(obj.material.diffuse));
			glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, glm::value_ptr(obj.material.specular));
			glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), obj.material.shininess);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, obj.textureID);
            
            glBindVertexArray(obj.vao);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
			glBindVertexArray(0);
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
        case GLFW_KEY_I: obj.position.y += 0.1f; break;
        case GLFW_KEY_J: obj.position.y -= 0.1f; break;
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
