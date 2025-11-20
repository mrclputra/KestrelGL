#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>  
using namespace std;

class Shader {
public:
  unsigned int ID;

  string vertexPath;
  string fragmentPath;
  string geometryPath;

  // last modification times
  time_t vertexModTime = 0;
  time_t fragmentModTime = 0;
  time_t geometryModTime = 0;

  // default
  Shader() : ID(0) {}

  // constructor
  Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    : vertexPath(vertexPath), fragmentPath(fragmentPath) {
    if (geometryPath) this->geometryPath = geometryPath;

    loadShaders();
    updateModTimes();
  }

  // check for file changes,
  // reload as needed
  // call this every frame
  bool checkAndReload() {
    bool modified = false;

    auto getModTime = [](const std::string& path) -> time_t {
      struct stat st;
      return (stat(path.c_str(), &st) == 0) ? st.st_mtime : 0;
    };

    time_t vMod = getModTime(vertexPath);
    time_t fMod = getModTime(fragmentPath);
    time_t gMod = geometryPath.empty() ? 0 : getModTime(geometryPath);

    if (vMod != vertexModTime || fMod != fragmentModTime || gMod != geometryModTime) {
      std::cout << "Updating Shaders...\n";
      loadShaders();
      updateModTimes();
      modified = true;
    }

    return modified;
  }

  // external calls
  // activate shader
  void use() {
    glUseProgram(ID);
  }
  // utility uniform functions
  void setBool(const std::string& name, bool value) const
  {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  void setInt(const std::string& name, int value) const
  {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setFloat(const std::string& name, float value) const
  {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setVec2(const std::string& name, const glm::vec2& value) const
  {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void setVec2(const std::string& name, float x, float y) const
  {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }
  void setVec3(const std::string& name, const glm::vec3& value) const
  {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void setVec3(const std::string& name, float x, float y, float z) const
  {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }
  void setVec4(const std::string& name, const glm::vec4& value) const
  {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void setVec4(const std::string& name, float x, float y, float z, float w)
  {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
  }
  void setMat2(const std::string& name, const glm::mat2& mat) const
  {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  void setMat3(const std::string& name, const glm::mat3& mat) const
  {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  void setMat4(const std::string& name, const glm::mat4& mat) const
  {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

private:
  void updateModTimes() {
    struct stat st;
    if (stat(vertexPath.c_str(), &st) == 0) vertexModTime = st.st_mtime;
    if (stat(fragmentPath.c_str(), &st) == 0) fragmentModTime = st.st_mtime;
    if (!geometryPath.empty() && stat(geometryPath.c_str(), &st) == 0)
      geometryModTime = st.st_mtime;
  }

  void loadShaders() {
    // retrieve source code from file path
    string vertexCode;
    string fragmentCode;
    string geometryCode;
    ifstream vShaderFile;
    ifstream fShaderFile;
    ifstream gShaderFile;

    // ensure ifstream objects can throw exceptions
    vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    gShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    try {
      // open files
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);
      // read file buffer contents into streams
      stringstream vShaderStream, fShaderStream;
      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();
      // close file handlers
      vShaderFile.close();
      fShaderFile.close();
      // convert stream into a string
      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
      // if geometry path present, load geometry shader
      if (!geometryPath.empty()) {
        gShaderFile.open(geometryPath);
        stringstream gShaderStream;
        gShaderStream << gShaderFile.rdbuf();
        gShaderFile.close();
        geometryCode = gShaderStream.str();
      }
    }
    catch (ifstream::failure& e) {
      std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << "\n";
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader, compile
    unsigned int geometry;
    if (!geometryPath.empty()) {
      const char* gShaderCode = geometryCode.c_str();
      geometry = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometry, 1, &gShaderCode, NULL);
      glCompileShader(geometry);
      checkCompileErrors(geometry, "GEOMETRY");
    }

    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (!geometryPath.empty())
      glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (!geometryPath.empty())
      glDeleteShader(geometry);
  }

  // compile error check utility function
  void checkCompileErrors(GLuint shader, string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- --" << std::endl;
      }
    }
    else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- --" << std::endl;
      }
    }
  }
};

#endif // !SHADER_HPP