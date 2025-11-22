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

  // default
  Model() : isGammaCorrection(false) {}

  // constructor
  // expects file path to a model file
  Model(string const& path, bool gamma = false) : isGammaCorrection(gamma) {
    loadModel(path);
  }

  // destructor
  ~Model() { cleanup(); }

  // prevent issues with copying
  // only one instance allowed
  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

  // allow for safe transfer of ownership
  Model(Model&& other) noexcept {
    *this = std::move(other);
  }
  Model& operator=(Model&& other) noexcept {
    if (this != &other) {
      cleanup();
      textures_loaded = std::move(other.textures_loaded);
      meshes = std::move(other.meshes);
      directory = std::move(other.directory);
      isGammaCorrection = other.isGammaCorrection;
    }
    return *this;
  }

  // draws the model and all it's meshes
  // transparency sorting
  void Draw(const Shader& shader, const glm::vec3& cameraPos, bool enableDepthWrite = true) {
    vector<size_t> opaqueMeshes;
    vector<size_t> transparentMeshes;

    for (size_t i = 0; i < meshes.size(); i++) {
      if (meshes[i].hasTransparency) {
        transparentMeshes.push_back(i);
      }
      else {
        opaqueMeshes.push_back(i);
      }
    }

    // draw opaque meshes first
    //glDepthMask(GL_TRUE);
    glDepthMask(enableDepthWrite ? GL_TRUE : GL_FALSE);
    for (size_t idx : opaqueMeshes) {
      meshes[idx].Draw(shader);
    }

    // skyboxes dont need transparency sorting
    if (!enableDepthWrite) return;

    // sort transparent meshes
    // back to front
    std::sort(transparentMeshes.begin(), transparentMeshes.end(),
      [this, &cameraPos](size_t a, size_t b) {
        float distA = glm::length(meshes[a].getCenter() - cameraPos);
        float distB = glm::length(meshes[b].getCenter() - cameraPos);
        return distA > distB; // farthest first
      });

    // draw transparent meshes 
    // without depth writing
    glDepthMask(GL_FALSE);
    for (size_t idx : transparentMeshes) {
      meshes[idx].Draw(shader);
    }
    glDepthMask(GL_TRUE); // restore depth writing
  }

  // gets center of model
  // uses AABB
  glm::vec3 getCenter() const {
    glm::vec3 minVertex(FLT_MAX), maxVertex(-FLT_MAX);

    // get farthest vertices
    for (const auto& mesh : meshes) {
      for (const auto& vertex : mesh.vertices) {
        minVertex = glm::min(minVertex, vertex.Position);
        maxVertex = glm::max(maxVertex, vertex.Position);
      }
    }

    // get center of those 2 vertices
    return (minVertex + maxVertex) * 0.5f;
  }

  // determines size from AABB
  glm::vec3 getSize() const {
    glm::vec3 minVertex(FLT_MAX), maxVertex(-FLT_MAX);

    for (const auto& mesh : meshes) {
      for (const auto& vertex : mesh.vertices) {
        minVertex = glm::min(minVertex, vertex.Position);
        maxVertex = glm::max(maxVertex, vertex.Position);
      }
    }

    return maxVertex - minVertex; // width, height, depth
  }

  // get scaling factor to get to a standard size
  float Model::getScaleToStandard(float standardSize) const {
    glm::vec3 size = getSize();
    float maxDim = glm::max(size.x, glm::max(size.y, size.z));
    return standardSize / maxDim; // scaling factor
  }

  void cleanup() {
    for (auto& texture : textures_loaded) {
      if (texture.id != 0) {
        glDeleteTextures(1, &texture.id);
      }
    }
    textures_loaded.clear();

    for (auto& mesh : meshes) {
      mesh.cleanup();
    }
    meshes.clear();
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
    //directory = path.substr(0, path.find_last_of('/'));
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != string::npos) {
      directory = path.substr(0, lastSlash);
    }
    else {
      directory = ".";
    }

    std::cout << "Model directory: " << directory << "\n"; // debug

    // process ASSIMP root node recursively
    processNode(scene->mRootNode, scene);

    std::cout << "Model Loaded\n";
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

    // debug stuff
    std::ostringstream vertexBuffer;
    const unsigned int flushInterval = 500; // flush
    unsigned int counter = 0;

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
        vector.y = mesh->mTangents[i].y;
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

      // debug ----
      vertexBuffer << "v" << i
        << " P[" << vertex.Position.x << " " << vertex.Position.y << " " << vertex.Position.z << "] "
        << "N[" << vertex.Normal.x << " " << vertex.Normal.y << " " << vertex.Normal.z << "] "
        << "T[" << vertex.Tangent.x << " " << vertex.Tangent.y << " " << vertex.Tangent.z << "] "
        << "B[" << vertex.Bitangent.x << " " << vertex.Bitangent.y << " " << vertex.Bitangent.z << "] "
        << "UV[" << vertex.TexCoords.x << " " << vertex.TexCoords.y << "]\n";
      counter++;

      if (counter >= flushInterval) {
        std::cout << vertexBuffer.str();
        vertexBuffer.str("");   // clear buffer
        vertexBuffer.clear();   // reset flags
        counter = 0;
      }
    }

    if (counter > 0) {
      std::cout << vertexBuffer.str();
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
    // array of textures

    // GLTF convention
    // https://www.khronos.org/files/gltf20-reference-guide.pdf
    
    // albedo maps
    vector<Texture> albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "ALBEDO");
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
    // metallicRoughness maps
    // encode in shader: metalness = B channel, roughness = G channel; ignore all other channels
    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_pbrmetallicroughness_metallicroughnesstexture
    vector<Texture> metallicRoughness = loadMaterialTextures(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, "METALLIC_ROUGHNESS");
    textures.insert(textures.end(), metallicRoughness.begin(), metallicRoughness.end());
    // normal maps
    vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "NORMAL");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // occlusion maps
    vector<Texture> occlusionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "OCCLUSION");
    textures.insert(textures.end(), occlusionMaps.begin(), occlusionMaps.end());
    // emission maps
    vector<Texture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "EMISSION");
    textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

    // TODO: handle specular model maps here
    // -

    // return object created from extracted mesh data
    return Mesh(vertices, indices, textures);
  }

  // check all material textures of a given type and loads textures if they are not loaded yet
  // required info is returned as a Texture struct
  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
    vector<Texture> textures;

    /*if (mat->GetTextureCount(type) > 0) {
      std::cout << "[MODEL] Found texture type: " << typeName
        << " (" << mat->GetTextureCount(type) << " textures)\n";
    }*/

    unsigned int count = mat->GetTextureCount(type);
    std::cout << "[TEXTURE] " << typeName << ": found " << count << " textures\n";

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

inline unsigned int TextureFromFile(const char* path, const string& directory, bool gamma) {
  string filename = string(path);
  //filename = directory + '/' + filename;

  bool isAbsolute = (filename[0] == '/' || filename[0] == '\\' ||
    (filename.length() > 1 && filename[1] == ':'));

  if (!isAbsolute) {
    filename = directory + '/' + filename;
  }

  std::cout << "Loading texture: " << filename << std::endl;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 2)
      format = GL_RG; // black and white + alpha
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;
    else
      std::cerr << "Warning: Unknown nrChannels=" << nrChannels << " for texture: " << filename << "\n";

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
