#include "model.h"

Model::Model(const char *path) {
    loadModel(path);
    // Initialize all bone transforms to identity
    boneTransforms.resize(100, glm::mat4(1.0f));
    for (int i = 0; i < 100; i++) {
        boneTransforms[i] = glm::mat4(1.0f);
    }
}

void Model::Draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::UpdateAnimation(float deltaTime) {
    if (!scene || !scene->mNumAnimations || !scene->mAnimations[0]) {
        // No animation available, skip
        return;
    }
    
    const aiAnimation* animation = scene->mAnimations[0];
    float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
    animationTime += deltaTime * ticksPerSecond;
    animationTime = fmod(animationTime, animation->mDuration);
    
    ReadNodeHierarchy(animationTime, scene->mRootNode, glm::mat4(1.0f));
}

std::vector<glm::mat4>& Model::GetBoneTransforms() {
    return boneTransforms;
}

void Model::loadModel(std::string path) {
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    
    globalInverseTransform = glm::inverse(ConvertMatrixToGLM(scene->mRootNode->mTransformation));
    
    if (scene->mNumAnimations > 0) {
        std::cout << "Found " << scene->mNumAnimations << " animations" << std::endl;
        std::cout << "Animation: " << scene->mAnimations[0]->mName.C_Str() << std::endl;
    }
    
    processNode(scene->mRootNode, scene);
    
    std::cout << "Total bones loaded: " << boneCounter << std::endl;
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        
        vertices.push_back(vertex);
    }
    
    // Extract bone weights
    ExtractBoneWeightForVertices(vertices, mesh, scene);
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    
    // Load material textures
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        
        // Load diffuse textures
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        // Load specular textures
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        // Check if texture was loaded before
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        
        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            std::cout << "Loaded texture: " << str.C_Str() << std::endl;
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const std::string &directory) {
    std::string filename = std::string(path);
    
    // Try multiple possible paths
    std::vector<std::string> possiblePaths = {
        "textures/" + filename,                    // textures/image.png
        directory + "/" + filename,                // model_directory/image.png
        directory + "/textures/" + filename,       // model_directory/textures/image.png
        filename                                   // Just the filename
    };
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = nullptr;
    std::string loadedPath;
    
    // Try each possible path until one works
    for (const auto& tryPath : possiblePaths) {
        data = stbi_load(tryPath.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            loadedPath = tryPath;
            break;
        }
    }
    
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        std::cout << "✓ Texture loaded successfully: " << loadedPath << std::endl;
    }
    else {
        std::cout << "✗ Texture failed to load at path: " << filename << std::endl;
        std::cout << "  Tried locations:" << std::endl;
        for (const auto& tryPath : possiblePaths) {
            std::cout << "    - " << tryPath << std::endl;
        }
    }
    
    return textureID;
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCounter;
            newBoneInfo.offset = ConvertMatrixToGLM(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCounter;
            boneCounter++;
        }
        else {
            boneID = boneInfoMap[boneName].id;
        }
        
        aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            
            if (vertexId >= vertices.size()) continue;
            
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                if (vertices[vertexId].BoneIDs[i] < 0) {
                    vertices[vertexId].Weights[i] = weight;
                    vertices[vertexId].BoneIDs[i] = boneID;
                    break;
                }
            }
        }
    }
    
    // Normalize weights to ensure they sum to 1.0
    for (auto& vertex : vertices) {
        float totalWeight = 0.0f;
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
            if (vertex.BoneIDs[i] >= 0) {
                totalWeight += vertex.Weights[i];
            }
        }
        
        if (totalWeight > 0.0f) {
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                if (vertex.BoneIDs[i] >= 0) {
                    vertex.Weights[i] /= totalWeight;
                }
            }
        }
    }
}

glm::mat4 Model::ConvertMatrixToGLM(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

void Model::ReadNodeHierarchy(float animationTime, const aiNode* node, const glm::mat4& parentTransform) {
    std::string nodeName(node->mName.data);
    const aiAnimation* animation = scene->mAnimations[0];
    glm::mat4 nodeTransformation = ConvertMatrixToGLM(node->mTransformation);
    
    const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);
    
    if (nodeAnim) {
        glm::vec3 position = InterpolatePosition(animationTime, nodeAnim);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        
        glm::quat rotation = InterpolateRotation(animationTime, nodeAnim);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
        
        glm::vec3 scale = InterpolateScale(animationTime, nodeAnim);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        
        nodeTransformation = translationMatrix * rotationMatrix * scaleMatrix;
    }
    
    glm::mat4 globalTransformation = parentTransform * nodeTransformation;
    
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        if (index >= 0 && index < boneTransforms.size()) {
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            boneTransforms[index] = globalInverseTransform * globalTransformation * offset;
        }
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ReadNodeHierarchy(animationTime, node->mChildren[i], globalTransformation);
    }
}

const aiNodeAnim* Model::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName) {
    for (unsigned int i = 0; i < animation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.data) == nodeName) {
            return nodeAnim;
        }
    }
    return nullptr;
}

glm::vec3 Model::InterpolatePosition(float animationTime, const aiNodeAnim* nodeAnim) {
    if (!nodeAnim || nodeAnim->mNumPositionKeys == 0) {
        return glm::vec3(0.0f);
    }
    
    if (nodeAnim->mNumPositionKeys == 1) {
        return glm::vec3(nodeAnim->mPositionKeys[0].mValue.x,
                       nodeAnim->mPositionKeys[0].mValue.y,
                       nodeAnim->mPositionKeys[0].mValue.z);
    }
    
    unsigned int positionIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
        if (animationTime < nodeAnim->mPositionKeys[i + 1].mTime) {
            positionIndex = i;
            break;
        }
    }
    
    unsigned int nextPositionIndex = (positionIndex + 1) % nodeAnim->mNumPositionKeys;
    float deltaTime = nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime;
    if (deltaTime <= 0.0f) deltaTime = 1.0f;
    float factor = (animationTime - nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    
    const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D& end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
    aiVector3D delta = end - start;
    
    return glm::vec3(start.x + factor * delta.x, start.y + factor * delta.y, start.z + factor * delta.z);
}

glm::quat Model::InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim) {
    if (!nodeAnim || nodeAnim->mNumRotationKeys == 0) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    
    if (nodeAnim->mNumRotationKeys == 1) {
        return glm::quat(nodeAnim->mRotationKeys[0].mValue.w,
                       nodeAnim->mRotationKeys[0].mValue.x,
                       nodeAnim->mRotationKeys[0].mValue.y,
                       nodeAnim->mRotationKeys[0].mValue.z);
    }
    
    unsigned int rotationIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        if (animationTime < nodeAnim->mRotationKeys[i + 1].mTime) {
            rotationIndex = i;
            break;
        }
    }
    
    unsigned int nextRotationIndex = (rotationIndex + 1) % nodeAnim->mNumRotationKeys;
    float deltaTime = nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime;
    if (deltaTime <= 0.0f) deltaTime = 1.0f;
    float factor = (animationTime - nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    
    const aiQuaternion& start = nodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
    
    aiQuaternion result;
    aiQuaternion::Interpolate(result, start, end, factor);
    result.Normalize();
    
    return glm::quat(result.w, result.x, result.y, result.z);
}

glm::vec3 Model::InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim) {
    if (!nodeAnim || nodeAnim->mNumScalingKeys == 0) {
        return glm::vec3(1.0f);
    }
    
    if (nodeAnim->mNumScalingKeys == 1) {
        return glm::vec3(nodeAnim->mScalingKeys[0].mValue.x,
                       nodeAnim->mScalingKeys[0].mValue.y,
                       nodeAnim->mScalingKeys[0].mValue.z);
    }
    
    unsigned int scaleIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
        if (animationTime < nodeAnim->mScalingKeys[i + 1].mTime) {
            scaleIndex = i;
            break;
        }
    }
    
    unsigned int nextScaleIndex = (scaleIndex + 1) % nodeAnim->mNumScalingKeys;
    float deltaTime = nodeAnim->mScalingKeys[nextScaleIndex].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime;
    if (deltaTime <= 0.0f) deltaTime = 1.0f;
    float factor = (animationTime - nodeAnim->mScalingKeys[scaleIndex].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    
    const aiVector3D& start = nodeAnim->mScalingKeys[scaleIndex].mValue;
    const aiVector3D& end = nodeAnim->mScalingKeys[nextScaleIndex].mValue;
    aiVector3D delta = end - start;
    
    return glm::vec3(start.x + factor * delta.x, start.y + factor * delta.y, start.z + factor * delta.z);
}
