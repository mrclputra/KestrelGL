#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
using namespace std;

#include <shader.hpp>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
    // bone indexes will ifluence this vertex
    // rigging
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

class Mesh {
public:
  // mesh data
  vector<Vertex>        vertices;
  vector<unsigned int>  indices;
  vector<Texture>       textures;
  unsigned int VAO;
  bool hasTransparency;

  // constructor
  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->hasTransparency = checkTransparency();

    setupMesh();
  }

  // render the mesh
  void Draw(const Shader& shader) {
    // pass to shader as array
    int diffuseCount = 0;
    int specularCount = 0;
    int normalCount = 0;
    int heightCount = 0;

    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);

      const string& type = textures[i].type;
      int index = 0;

      if (type == "DIFFUSE") index = diffuseCount++;
      else if (type == "SPECULAR") index = specularCount++;
      else if (type == "NORMAL") index = normalCount++;
      else if (type == "HEIGHT") index = heightCount++;

      // TODO: switch to bindless textures or sampler2Darray
      // in shader, call: ex. DIFFUSE[0]
      string uniformName = type + "[" + std::to_string(index) + "]";

      glUniform1i(glGetUniformLocation(shader.ID, uniformName.c_str()), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    shader.setInt("numDiffuse", diffuseCount);
    shader.setInt("numSpecular", specularCount);
    shader.setInt("numNormal", normalCount);
    shader.setInt("numHeight", heightCount);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
  }

  glm::vec3 getCenter() const {
    glm::vec3 center(0.0f);
    for (const auto& v : vertices) {
      center += v.Position;
    }
    return center / static_cast<float>(vertices.size());
  }

  void cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

private:
  // render data
  unsigned int VBO, EBO; // vertex, index buffer

  bool checkTransparency() {
    for (const auto& tex : textures) {
      if (tex.type == "DIFFUSE") {
        // bind and check alpha channel
        glBindTexture(GL_TEXTURE_2D, tex.id);

        int width, height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        // sample a few pixels to check for alpha < 1.0
        // we assume RGBA textures means having transparency
        GLint format;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

        if (format == GL_RGBA || format == GL_RGBA8 || format == GL_RGBA16) {
          // has alpha channel
          return true; 
        }
      }
    }
    return false;
  }

  void setupMesh() {
    // create buffers and arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // note that in structs, memory layout is sequential for all items
    // we can pass a pointer to a struct and it can translate into a glm::vec3/2 array
    // which translates into 3/2 floats which translates to a byte array
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // set vertex buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // set index buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set vertex attribute pointers
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
  }
};

#endif // !MESH_HPP
