#ifndef MODEL_HPP
#define MODEL_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mesh.hpp>
#include <shader.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model {
public:
  // model data
  vector<Texture> textures_loaded;
  vector<Mesh> meshes;
  string directory;
  bool isGammaCorrection;

  // constructor
  // expects file path to a model file
  Model(string const& path, bool gamma = false) : isGammaCorrection(gamma) {
    loadModel(path);
    std::cout << "Model Successfully Loaded...\n";
  }

  // draws the model and all it's meshes
  void Draw(Shader shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
      meshes[i].Draw(shader);
    }
  }

private:
  // loads a model with supported file extensions w/ ASSIMP
  // stores resulting meshes in the above mesh vector
  void loadModel(string const& path) {
    // read file
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      std::cerr << "ERROR:ASSIMP:: " << importer.GetErrorString() << "\n";
      return;
    }

    // retrieve directory of the file path
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP root node recursively
    processNode(scene->mRootNode, scene);
  }

  // processes current node, and children (if any)
  void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      // node object only contains indices to index the actual objects in the scene
      // cross-references with vertices in scene to build the model
      // nodes are just to keep stuff organized
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh, scene));
    }

    // children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }

  Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      Vertex vertex;
      glm::vec3 vector; // 3D placeholder vector, since assimp uses it's own vector class that doesn't directly convert to OpenGL
      // positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.Position = vector;

      // normals
      if (mesh->HasNormals()) {
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
      }

      // texture coordinates
      if (mesh->mTextureCoords[0]) {
        glm::vec2 vec; // 2D placeholder

        // a vertex can have up to 8 different texture coordinates
        // we cant assume that models wont have multiple texture coordinates, so we take the first set (0)
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = 1.0f - mesh->mTextureCoords[0][i].y;
        vertex.TexCoords = vec;
        // tangent
        vector.x = mesh->mTangents[i].x;
        vector.z = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.Tangent = vector;
        // bitangent
        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        vertex.Bitangent = vector;
      }
      else {
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
      }

      vertices.push_back(vertex);
    }

    // go through each face and retrieve corresponding indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        indices.push_back(face.mIndices[j]);
      }
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders
    // each diffuse texture should be names as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER
    // same applies to other texture types:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // normal maps
    vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // height maps
    vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return object created from extracted mesh data
    return Mesh(vertices, indices, textures);
  }

  // check all material textures of a given type and loads textures if they are not loaded yet
  // required info is returned as a Texture struct
  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);
      // check if texture was loaded before, and if so, continue to next iteration
      // some meshes share texture maps
      bool skip = false;
      for (unsigned int j = 0; j < textures_loaded.size(); j++) {
        if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
          textures.push_back(textures_loaded[j]);
          skip = true;
          break;
        }
      }
      if (!skip) {  // texture has not been loaded yet
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), this->directory);
        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
        textures_loaded.push_back(texture); // entire model
      }
    }

    return textures;
  }
};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma) {
  string filename = string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else {
    std::cout << "Failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}


#endif // !MODEL_HPP
