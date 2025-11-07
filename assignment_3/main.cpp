#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "mesh.h"
#include "model.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// ===================== Game Object =====================
struct GameObject {
    glm::vec3 position;
    glm::vec3 scale;
    float rotation;
    Model* model;
    float boundingRadius;
    bool active;
    
    GameObject(Model* m, glm::vec3 pos, float rad = 1.0f) 
    : model(m), position(pos), scale(1.0f), rotation(0.0f), boundingRadius(rad), active(true) {}
    
    void draw(Shader& shader) {
        if (!active) return;
        
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::rotate(modelMat, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::scale(modelMat, scale);
        shader.setMat4("model", modelMat);
        model->Draw(shader);
    }
    
    bool checkCollision(const GameObject& other) {
        if (!active || !other.active) return false;
        float distance = glm::length(position - other.position);
        return distance < (boundingRadius + other.boundingRadius);
    }
};

// ===================== Camera =====================
class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float distance;
    float height;
    
    Camera(glm::vec3 pos = glm::vec3(0.0f, 5.0f, 10.0f)) 
    : position(pos), target(0.0f), up(0.0f, 1.0f, 0.0f), distance(10.0f), height(5.0f) {}
    
    void followTarget(glm::vec3 targetPos) {
        target = targetPos;
        position = target - glm::vec3(0.0f, -height, distance);
    }
    
    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, target, up);
    }
};

// ===================== Global Variables =====================
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Input
bool keys[1024];
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

// ===================== Simple Cube Model Generator =====================
Model* createCubeModel() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Simple cube vertices
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f
    };
    
    for (int i = 0; i < 8; i++) {
        Vertex v;
        v.Position = glm::vec3(cubeVertices[i*8], cubeVertices[i*8+1], cubeVertices[i*8+2]);
        v.Normal = glm::vec3(cubeVertices[i*8+3], cubeVertices[i*8+4], cubeVertices[i*8+5]);
        v.TexCoords = glm::vec2(cubeVertices[i*8+6], cubeVertices[i*8+7]);
        vertices.push_back(v);
    }
    
    unsigned int cubeIndices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1
    };
    
    for (int i = 0; i < 36; i++) {
        indices.push_back(cubeIndices[i]);
    }
    
    std::vector<Texture> textures;
    std::vector<Mesh> meshes;
    meshes.push_back(Mesh(vertices, indices, textures));
    
    Model* model = new Model("");
    model->meshes = meshes;
    return model;
}

Model* loadModelFromFile(const char* filepath) {
    std::cout << "========================================" << std::endl;
    std::cout << "Attempting to load model from: " << filepath << std::endl;
    
    Model* model = nullptr;
    try {
        model = new Model(filepath);
        
        if (model->meshes.empty()) {
            std::cout << "ERROR: Model loaded but contains no meshes!" << std::endl;
            delete model;
            model = nullptr;
        } else {
            std::cout << "SUCCESS: Model loaded!" << std::endl;
            std::cout << "Number of meshes: " << model->meshes.size() << std::endl;
            return model;
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: Exception while loading model: " << e.what() << std::endl;
        if (model) delete model;
        model = nullptr;
    }
    
    std::cout << "Creating fallback cube model..." << std::endl;
    return createCubeModel();
}

// ===================== Main Function =====================
int main() {
    srand(time(NULL));
    
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Adventure Game", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);
    
    // Create shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;
        layout (location = 3) in ivec4 aBoneIDs;
        layout (location = 4) in vec4 aWeights;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat4 boneTransforms[100];
        uniform bool hasAnimation;
        
        void main() {
            vec4 totalPosition = vec4(0.0);
            vec3 totalNormal = vec3(0.0);
            float totalWeight = 0.0;
            
            if(hasAnimation) {
                for(int i = 0; i < 4; i++) {
                    if(aBoneIDs[i] == -1) continue;
                    if(aBoneIDs[i] >= 100) {
                        totalPosition = vec4(aPos, 1.0);
                        totalNormal = aNormal;
                        totalWeight = 1.0;
                        break;
                    }
                    vec4 localPosition = boneTransforms[aBoneIDs[i]] * vec4(aPos, 1.0);
                    totalPosition += localPosition * aWeights[i];
                    vec3 localNormal = mat3(boneTransforms[aBoneIDs[i]]) * aNormal;
                    totalNormal += localNormal * aWeights[i];
                    totalWeight += aWeights[i];
                }
                
                if(totalWeight == 0.0) {
                    totalPosition = vec4(aPos, 1.0);
                    totalNormal = aNormal;
                }
            } else {
                totalPosition = vec4(aPos, 1.0);
                totalNormal = aNormal;
            }
            
            FragPos = vec3(model * totalPosition);
            Normal = mat3(transpose(inverse(model))) * totalNormal;
            TexCoords = aTexCoords;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        
        uniform vec3 objectColor;
        uniform vec3 lightPos;
        uniform vec3 viewPos;
        uniform sampler2D texture_diffuse;
        uniform bool useTexture;
        
        void main() {
            vec3 baseColor;
            if (useTexture) {
                baseColor = texture(texture_diffuse, TexCoords).rgb;
            } else {
                baseColor = objectColor;
            }
            
            vec3 ambient = 0.3 * baseColor;
            
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * baseColor;
            
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = 0.5 * spec * vec3(1.0);
            
            vec3 result = ambient + diffuse + specular;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Create simple cube model
    Model* cubeModel = createCubeModel();
    
    // Create game objects
    Model* playerModel = loadModelFromFile("Swimming.dae");
    GameObject player(playerModel, glm::vec3(0.0f, 0.5f, 0.0f), 0.8f);
    player.scale = glm::vec3(0.01f, 0.01f, 0.01f);

    std::cout << "Player model loaded. Meshes: " << playerModel->meshes.size() << std::endl;
    std::cout << "Textures loaded: " << playerModel->textures_loaded.size() << std::endl;
    
    std::vector<GameObject> obstacles;
    obstacles.push_back(GameObject(cubeModel, glm::vec3(5.0f, 0.5f, 0.0f), 1.0f));
    obstacles.push_back(GameObject(cubeModel, glm::vec3(-5.0f, 0.5f, 5.0f), 1.0f));
    obstacles.push_back(GameObject(cubeModel, glm::vec3(0.0f, 0.5f, -8.0f), 1.0f));
    
    std::vector<GameObject> collectibles;
    for (int i = 0; i < 5; i++) {
        float x = (rand() % 20 - 10);
        float z = (rand() % 20 - 10);
        collectibles.push_back(GameObject(cubeModel, glm::vec3(x, 0.5f, z), 0.5f));
        collectibles.back().scale = glm::vec3(0.5f);
    }
    
    // Create ground
    GameObject ground(cubeModel, glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
    ground.scale = glm::vec3(30.0f, 0.5f, 30.0f);
    
    // Camera
    Camera camera;
    
    int score = 0;
    float playerSpeed = 5.0f;
    
    // Game loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Update player animation
        playerModel->UpdateAnimation(deltaTime);
        
        // Input processing (camera-relative movement)
        glm::vec3 moveDirection(0.0f);

        // Calculate camera's forward and right vectors
        glm::vec3 cameraForward = glm::normalize(player.position - camera.position);
        cameraForward.y = 0.0f;
        if (glm::length(cameraForward) > 0.0f) {
            cameraForward = glm::normalize(cameraForward);
        }
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraForward, glm::vec3(0.0f, 1.0f, 0.0f)));

        // ESC to exit
        if (keys[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(window, true);
        }

        // WASD movement relative to camera
        if (keys[GLFW_KEY_W]) moveDirection += cameraForward;
        if (keys[GLFW_KEY_S]) moveDirection -= cameraForward;
        if (keys[GLFW_KEY_A]) moveDirection -= cameraRight;
        if (keys[GLFW_KEY_D]) moveDirection += cameraRight;

        // Apply movement
        if (glm::length(moveDirection) > 0.0f) {
            moveDirection = glm::normalize(moveDirection);
            glm::vec3 newPos = player.position + moveDirection * playerSpeed * deltaTime;

            // Check collision with obstacles
            bool collision = false;
            GameObject tempPlayer = player;
            tempPlayer.position = newPos;

            for (auto& obstacle : obstacles) {
                if (tempPlayer.checkCollision(obstacle)) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                player.position = newPos;
            }

            // Rotate player to face movement direction
            player.rotation = atan2(moveDirection.x, moveDirection.z);
        }

        // Check collectibles
        for (auto& collectible : collectibles) {
            if (player.checkCollision(collectible)) {
                collectible.active = false;
                score++;
                std::cout << "Score: " << score << std::endl;
            }
        }

        // Update camera
        camera.followTarget(player.position);
        
        // Rendering
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(glm::vec3(10.0f, 10.0f, 10.0f)));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.position));
        
        // Draw ground
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.3f, 0.5f, 0.3f);
        glUniform1i(glGetUniformLocation(shaderProgram, "hasAnimation"), false);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ground.position);
        model = glm::scale(model, ground.scale);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        ground.model->Draw(*((Shader*)&shaderProgram));
        
        // Draw player with animation
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.5f, 0.9f);
        glUniform1i(glGetUniformLocation(shaderProgram, "hasAnimation"), true);
        
        // Set bone transforms
        std::vector<glm::mat4>& transforms = playerModel->GetBoneTransforms();
        for (unsigned int i = 0; i < transforms.size(); i++) {
            std::string uniformName = "boneTransforms[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, player.position);
        model = glm::rotate(model, player.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, player.scale);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        player.model->Draw(*((Shader*)&shaderProgram));
        
        // Draw obstacles
        glUniform1i(glGetUniformLocation(shaderProgram, "hasAnimation"), false);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.2f, 0.2f);
        for (auto& obstacle : obstacles) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, obstacle.position);
            model = glm::scale(model, obstacle.scale);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            obstacle.model->Draw(*((Shader*)&shaderProgram));
        }
        
        // Draw collectibles
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.8f, 0.0f);
        for (auto& collectible : collectibles) {
            if (collectible.active) {
                collectible.rotation += deltaTime * 2.0f;
                model = glm::mat4(1.0f);
                model = glm::translate(model, collectible.position);
                model = glm::rotate(model, collectible.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, collectible.scale);
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                collectible.model->Draw(*((Shader*)&shaderProgram));
            }
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    delete cubeModel;
    delete playerModel;
    
    glfwTerminate();
    return 0;
}
