#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;
    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    glm::mat4 globalInverseTransform;
    std::vector<glm::mat4> boneTransforms;
    float animationTime = 0.0f;
    
    Model(const char *path);
    void Draw(Shader &shader);
    void UpdateAnimation(float deltaTime);
    std::vector<glm::mat4>& GetBoneTransforms();
    
private:
    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char *path, const std::string &directory);
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
    glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& from);
    void ReadNodeHierarchy(float animationTime, const aiNode* node, const glm::mat4& parentTransform);
    const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
    glm::vec3 InterpolatePosition(float animationTime, const aiNodeAnim* nodeAnim);
    glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
    glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);
};

#endif
