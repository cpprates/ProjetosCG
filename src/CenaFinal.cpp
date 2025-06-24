#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
using namespace glm;

struct Vertex
{
    vec3 position;
    vec2 texCoord;
    vec3 normal;
};

struct Material {
    vec3 ka = vec3(0.1f);
    vec3 kd = vec3(1.0f);
    vec3 ks = vec3(0.5f);
    float shininess = 32.0f;
};

struct Modelo
{
    GLuint VAO, VBO, textura;
    int vertexCount;
    Material material;
};

vec3 ka(0.1f), kd(1.0f), ks(0.5f);
float shininess = 32.0f;
vec3 lightColor;
vec3 lightPos;
GLuint shaderProgram;
Modelo ovni, vaca, casa, chao;
GLuint skyboxTexture, quadVAO;
GLuint skyboxShader;

bool casaLuz = false;
float ovniY = 5.0f, vacaY = 0.0f;
float alturaAbducao = 5.0f, alturaFuga = 15.0f;
float vacaX = 0.0f;
float curvaAmplitude = 1.0f; // raio da curva no plano XZ

// ============== CAMERA ==============
class Camera
{
public:
    vec3 position, front, up, right, worldUp;
    float yaw, pitch, speed, sensitivity;

    Camera(vec3 pos) : position(pos), worldUp(0.0f, 1.0f, 0.0f), yaw(-90.0f), pitch(0.0f), speed(2.5f), sensitivity(0.1f)
    {
        updateCameraVectors();
    }

    mat4 GetViewMatrix()
    {
        return lookAt(position, position + front, up);
    }

    void Move(string dir, float deltaTime)
    {
        float velocity = speed * deltaTime;
        if (dir == "FORWARD")
            position += front * velocity;
        if (dir == "BACKWARD")
            position -= front * velocity;
        if (dir == "LEFT")
            position -= right * velocity;
        if (dir == "RIGHT")
            position += right * velocity;
    }

    void Rotate(float xoffset, float yoffset)
    {
        xoffset *= sensitivity;
        yoffset *= sensitivity;
        yaw += xoffset;
        pitch += yoffset;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        vec3 f;
        f.x = cos(radians(yaw)) * cos(radians(pitch));
        f.y = sin(radians(pitch));
        f.z = sin(radians(yaw)) * cos(radians(pitch));
        front = normalize(f);
        right = normalize(cross(front, worldUp));
        up = normalize(cross(right, front));
    }
};

Camera camera(vec3(0.0f, 1.5f, 10.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;
float lastX = 400.0f, lastY = 300.0f;

// ============== SHADERS ==============
const char *vertexShaderSource = R"(
    #version 450 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texCoord;
    layout(location = 2) in vec3 normal;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = texCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
})";

const char* fragmentShaderSource = R"(
   #version 450 core
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D texBuff;
    uniform vec3 ka, kd, ks;
    uniform float shininess;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 lightDir;
    uniform vec3 viewPos;

    void main() {
    vec3 ambient = ka * texture(texBuff, TexCoord).rgb;

    vec3 baseColor = texture(texBuff, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPos - FragPos);

    // Spot cutoff
    float theta = dot(lightDirection, normalize(-lightDir));
    float cutoff = 0.85;
    float outerCutoff = 0.70;
    float intensity = clamp((theta - outerCutoff) / (cutoff - outerCutoff), 0.0, 1.0);

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = kd * diff * baseColor * lightColor * intensity;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specColor = lightColor * 0.4 + vec3(0.2);
    vec3 specular = ks * spec * specColor * intensity;

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.01 * distance * distance);

    vec3 result = (ambient + diffuse + specular) * attenuation * 2.0;
    FragColor = vec4(result, 1.0);
}
    )";

const char *skyboxVertex = R"(
    #version 450 core
    out vec2 TexCoord;
    void main() {
        vec2 pos[6] = vec2[](
            vec2(-1, 1), vec2(-1, -1), vec2(1, -1),
            vec2(-1, 1), vec2(1, -1), vec2(1, 1)
        );
        vec2 tex[6] = vec2[](
            vec2(0, 1), vec2(0, 0), vec2(1, 0),
            vec2(0, 1), vec2(1, 0), vec2(1, 1)
        );
        gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
        TexCoord = tex[gl_VertexID];
    }
    )";

const char *skyboxFragment = R"(
    #version 450 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D skyTexture;
    void main() {
        FragColor = texture(skyTexture, TexCoord);
    }
    )";

GLuint compileSkyboxShader()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &skyboxVertex, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &skyboxFragment, NULL);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

GLuint compileShader()
{
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vertexShaderSource, NULL);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fragmentShaderSource, NULL);
    glCompileShader(f);
    GLuint program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);
    glDeleteShader(v);
    glDeleteShader(f);
    return program;
}

GLuint loadTexture(const string &path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

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

void initSkybox()
{
    glGenVertexArrays(1, &quadVAO);
    skyboxTexture = loadTexture("../assets/Modelos3D/final/ceu.png");
    skyboxShader = compileSkyboxShader();
}

bool loadOBJWithMTL(const string &objPath, const string &mtlDir, vector<Vertex> &outVertices, GLuint &textureID, Material &material)
{
    ifstream file(objPath);
    if (!file.is_open())
        return false;

    vector<vec3> positions, normals;
    vector<vec2> texCoords;
    map<string, string> materials;
    string activeTexture, line;
    float Ns;

    while (getline(file, line))
    {
        istringstream iss(line);
        string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            float x, y, z;
            iss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        }
        else if (prefix == "vt")
        {
            float u, v;
            iss >> u >> v;
            texCoords.emplace_back(u, 1.0f - v);  // Corrige o eixo Y
        }
        else if (prefix == "vn")
        {
            float x, y, z;
            iss >> x >> y >> z;
            normals.emplace_back(x, y, z);
        }
        else if (prefix == "f")
        {
            string v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            string vs[] = {v1, v2, v3};
            for (auto &v : vs)
            {
                int p = 0, t = 0, n = 0;
                sscanf(v.c_str(), "%d/%d/%d", &p, &t, &n);
                outVertices.push_back({positions[p - 1], texCoords[t - 1], normals[n - 1]});
            }
        }
        else if (prefix == "mtllib")
        {
            string mtlFile;
            iss >> mtlFile;
            ifstream mtl(mtlDir + "/" + mtlFile);
            string mline, matName;
            while (getline(mtl, mline))
            {
                istringstream mss(mline);
                string tag;
                mss >> tag;
                if (tag == "newmtl")
                    mss >> matName;
                else if (tag == "map_Kd")
                    mss >> materials[matName];
                else if (tag == "Ka")
                    mss >> material.ka.r >> material.ka.g >> material.ka.b;
                else if (tag == "Kd")
                    mss >> material.kd.r >> material.kd.g >> material.kd.b;
                else if (tag == "Ks")
                    mss >> material.ks.r >> material.ks.g >> material.ks.b;
                else if (tag == "Ns")
                    mss >> material.shininess;
            }
        }
        else if (prefix == "usemtl")
        {
            string name;
            iss >> name;
            if (materials.count(name))
            {
                activeTexture = materials[name];
                textureID = loadTexture("../assets/Modelos3D/final/" + activeTexture);
            }
        }
    }

    return !outVertices.empty();
}

bool loadModel(const string &path, Modelo &modelo)
{
    vector<Vertex> verts;
    if (!loadOBJWithMTL(path, "../assets/Modelos3D/final", verts, modelo.textura, modelo.material))
        return false;
    modelo.vertexCount = verts.size();
    glGenVertexArrays(1, &modelo.VAO);
    glGenBuffers(1, &modelo.VBO);
    glBindVertexArray(modelo.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, modelo.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    return true;
}

void mouse_callback(GLFWwindow *, double xpos, double ypos)
{
    static bool first = true;
    if (first)
    {
        lastX = xpos;
        lastY = ypos;
        first = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.Rotate(xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
    float current = glfwGetTime();
    deltaTime = current - lastFrame;
    lastFrame = current;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Move("FORWARD", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Move("BACKWARD", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Move("LEFT", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Move("RIGHT", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        casaLuz = !casaLuz;
        glfwWaitEventsTimeout(0.2);
    }
}

int main() {
    glfwInit();
    GLFWwindow* w = glfwCreateWindow(800, 600, "OVNI vs Vaca", NULL, NULL);
    glfwMakeContextCurrent(w);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(w, mouse_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 2.0f);

    shaderProgram = compileShader();
    initSkybox();

    loadModel("../assets/Modelos3D/final/Nave.obj", ovni);
    loadModel("../assets/Modelos3D/final/vaca.obj", vaca);
    loadModel("../assets/Modelos3D/final/casa.obj", casa);

    // ==== CHÃO ====
    vector<Vertex> chaoVerts = {
        {{-50.0f, 0.0f, -50.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 50.0f, 0.0f, -50.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 50.0f, 0.0f,  50.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-50.0f, 0.0f, -50.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 50.0f, 0.0f,  50.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-50.0f, 0.0f,  50.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    };
    chao.vertexCount = chaoVerts.size();
    chao.textura = loadTexture("../assets/Modelos3D/final/grama.png");
    chao.material.ka = vec3(0.2f);
    chao.material.kd = vec3(0.8f);
    chao.material.ks = vec3(0.1f);
    chao.material.shininess = 8.0f;
    glGenVertexArrays(1, &chao.VAO);
    glGenBuffers(1, &chao.VBO);
    glBindVertexArray(chao.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, chao.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * chaoVerts.size(), chaoVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    // ==== ESTADOS INICIAIS ====
    casaLuz = true;
    const float ovniTopo = alturaFuga + 5.0f;
    const float ovniBaixo = alturaAbducao + 1.5f;
    ovniY = ovniTopo;
    vacaY = 0.0f;
    float vacaRot = 0.0f;

    while (!glfwWindowShouldClose(w)) {
        processInput(w);
        glfwPollEvents();
        float t = glfwGetTime();

        float baseSpeed = 1.5f;
        float vacaDist = alturaAbducao - vacaY;
        float ovniDist = ovniY - ovniBaixo;
        float ovniSpeed = (vacaDist > 0.01f && ovniDist > 0.01f) ? baseSpeed * (ovniDist / vacaDist) : baseSpeed;

        if (casaLuz) {
            if (ovniY < ovniTopo)
                ovniY += ovniSpeed * deltaTime;
            else
                ovniY = ovniTopo;
            if (vacaY > 0.0f) {
                vacaY -= baseSpeed * deltaTime;

                // Aplica rotação decrescente na vaca durante a queda
                if (vacaY < alturaAbducao) {
                    vacaRot += 5.0f * deltaTime;  // controla a velocidade da rotação
                    if (vacaRot > glm::radians(720.0f)) // no máximo 2 voltas
                        vacaRot = glm::radians(720.0f);
                }
            } else {
                vacaY = 0.0f;
                vacaRot = 0.0f; // reseta rotação quando toca o chão
            }            
        } else {
            if (ovniY > ovniBaixo)
                ovniY -= ovniSpeed * deltaTime;
            else
                ovniY = ovniBaixo;

            if (vacaY < alturaAbducao) {
                vacaY += baseSpeed * deltaTime;
                vacaX = sin(t * 2.0f) * 0.5f; // curva suave em X
            } else {
                vacaY = alturaAbducao;
                vacaX = 0.0f;
            }
        }

        // ==== DESENHO DO FUNDO ====
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(skyboxShader);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        // ==== SHADER PRINCIPAL ====
        glUseProgram(shaderProgram);
        mat4 proj = perspective(radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(proj));

        // === AJUSTE DE MATERIAIS E LUZ ===
        vec3 vacaPos = vec3(0, vacaY, 0);

        if (casaLuz) {
            ka = vec3(0.2f);
            kd = vec3(1.5f);
            ks = vec3(0.3f);
            lightColor = vec3(1.0f);
            lightPos = vec3(5.0f, 1.5f, -6.5f); // dentro da casa
            vec3 dir = normalize(vacaPos - lightPos);
            glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, value_ptr(dir));
        } else {
            ka = vec3(0.05f, 0.2f, 0.05f);
            kd = vec3(0.2f, 1.0f, 0.2f);
            ks = vec3(0.1f, 0.8f, 0.1f);
            lightColor = vec3(0.0f, 1.0f, 0.0f);
            lightPos = vec3(0, ovniY - 1.0f, 0);
            vec3 dir = normalize(vec3(0, -1, 0));
            glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, value_ptr(dir));
        }

        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, value_ptr(camera.position));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, value_ptr(lightColor));

        // ==== DESENHO ====
        auto draw = [&](Modelo& m, mat4 model) {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderProgram, "ka"), 1, value_ptr(m.material.ka));
            glUniform3fv(glGetUniformLocation(shaderProgram, "kd"), 1, value_ptr(m.material.kd));
            glUniform3fv(glGetUniformLocation(shaderProgram, "ks"), 1, value_ptr(m.material.ks));
            glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), m.material.shininess);
            glBindVertexArray(m.VAO);
            glBindTexture(GL_TEXTURE_2D, m.textura);
            glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
        };

        draw(chao, mat4(1.0f));
        draw(ovni, translate(mat4(1.0f), vec3(0, ovniY, 0)) * rotate(mat4(1.0f), t, vec3(0, 1, 0)));
        draw(casa, translate(mat4(1.0f), vec3(5, 0, -5)));
        // draw(vaca, translate(mat4(1.0f), vec3(vacaX, vacaY, 0)));

        vec3 posVaca;
        if (!casaLuz && vacaY >= alturaAbducao) {
            float curvaT = t * 2.0f; // velocidade da curva
            float vacaX = curvaAmplitude * sin(curvaT);
            float vacaZ = curvaAmplitude * sin(curvaT) * cos(curvaT);
            posVaca = vec3(vacaX, vacaY, vacaZ);
        } else {
            posVaca = vec3(vacaX, vacaY, 0);
        }
        
        mat4 modelVaca = translate(mat4(1.0f), posVaca);
        // Aplica rotação na vaca apenas quando caindo (casaLuz == true)
        if (casaLuz && vacaY < alturaAbducao && vacaY > 0.0f)
            modelVaca = modelVaca * rotate(mat4(1.0f), vacaRot, vec3(1, 0, 0));

        draw(vaca, modelVaca);
        // draw(vaca, translate(mat4(1.0f), posVaca));

        glfwSwapBuffers(w);
    }

    glfwTerminate();
    return 0;
}