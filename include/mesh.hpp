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

  // constructor
  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
  }

  // render the mesh
  void Draw(const Shader& shader) {
    if (textures.empty()) {
      shader.setBool("hasDiffuseMap", false);
    }
    else {
      shader.setBool("hasDiffuseMap", true);
    }

    bool hasNormalMap = false;
    for (auto& tex : textures) {
      if (tex.type == "NORMAL") {
        hasNormalMap = true;
        break;
      }
    }
    shader.setBool("hasNormalMap", hasNormalMap);

    bool hasSpecularMap = false;
    for (auto& tex : textures) {
      if (tex.type == "SPECULAR") {
        hasSpecularMap = true;
        break;
      }
    }
    shader.setBool("hasSpecularMap", hasSpecularMap);

    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i); // activate texture before binding
      // retrieve texture number (N in diffuse_textureN)
      string number;
      string name = textures[i].type;

      // assign number for shader uniform
      if (name == "DIFFUSE")
        number = std::to_string(diffuseNr++);
      else if (name == "SPECULAR")
        number = std::to_string(specularNr++);
      else if (name == "NORMAL")
        number = std::to_string(normalNr++);
      else if (name == "HEIGHT")
        number = std::to_string(heightNr++);

      // bind to correct uniform name in shader
      glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id); // bind texture
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
  }

  void cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

private:
  // render data
  unsigned int VBO, EBO; // vertex, index buffer

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
