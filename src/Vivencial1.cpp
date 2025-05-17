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

// Shader (vertex/fragment) igual ao já existente
extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

GLuint setupShader();

int loadSimpleOBJ(string filePath, int &nVertices)
{
	vector <glm::vec3> vertices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vBuffer;

	glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);

	ifstream arqEntrada;

	arqEntrada.open(filePath.c_str());
	if (arqEntrada.is_open())
	{
		//Fazer o parsing
		string line;
		while (!arqEntrada.eof())
		{
			getline(arqEntrada,line);
			istringstream ssline(line);
			string word;
			ssline >> word;
			if (word == "v")
			{
				glm::vec3 vertice;
				ssline >> vertice.x >> vertice.y >> vertice.z;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				vertices.push_back(vertice);

			}
			if (word == "vt")
			{
				glm::vec2 vt;
				ssline >> vt.s >> vt.t;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				texCoords.push_back(vt);

			}
			if (word == "vn")
			{
				glm::vec3 normal;
				ssline >> normal.x >> normal.y >> normal.z;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				normals.push_back(normal);

			}
			else if (word == "f")
			{
				while (ssline >> word) 
				{
					int vi, ti, ni;
					istringstream ss(word);
    				std::string index;

    				// Pega o índice do vértice
    				std::getline(ss, index, '/');
    				vi = std::stoi(index) - 1;  // Ajusta para índice 0

    				// Pega o índice da coordenada de textura
    				std::getline(ss, index, '/');
    				ti = std::stoi(index) - 1;

    				// Pega o índice da normal
    				std::getline(ss, index);
    				ni = std::stoi(index) - 1;

					//Recuperando os vértices do indice lido
					vBuffer.push_back(vertices[vi].x);
					vBuffer.push_back(vertices[vi].y);
					vBuffer.push_back(vertices[vi].z);
					
					//Atributo cor
					vBuffer.push_back(color.r);
					vBuffer.push_back(color.g);
					vBuffer.push_back(color.b);
					
        			
        			// Exibindo os índices para verificação
       				// std::cout << "v: " << vi << ", vt: " << ti << ", vn: " << ni << std::endl;
    			}
				
			}
		}

		arqEntrada.close();

		cout << "Gerando o buffer de geometria..." << endl;
		GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	nVertices = vBuffer.size() / 2;
	return VAO;

	}
	else
	{
		cout << "Erro ao tentar ler o arquivo " << filePath << endl;
		return -1;
	}
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CG - Carolina Prates , Kevin Kuhn, Vitor Mello", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = setupShader();
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    // Carregar objetos
    vector<string> paths = {
        "../assets/Modelos3D/Suzanne.obj",
        "../assets/Modelos3D/Cube.obj"
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
