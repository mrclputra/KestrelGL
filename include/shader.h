#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

// TODO: move implementation to components folder
//  and separate into header vs implementation

/// <summary>
/// This class encapsulates an OpenGL shader program,
/// handling loading, compiling, and setting uniform variables.
/// </summary>
class Shader {
public:
    unsigned int ID = 0;

    Shader() = default;

    /// <summary>
    /// Construct a shader program from vertex, fragment, and optional geometry shader files.
    /// </summary>
    /// <param name="vertexPath">Path to the vertex shader.</param>
    /// <param name="fragmentPath">Path to the fragment shader.</param>
    /// <param name="geometryPath">Optional path to the geometry shader.</param>
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) 
        : m_vertexPath(vertexPath), m_fragmentPath(fragmentPath), m_geometryPath(geometryPath ? geometryPath : "") {
        compile();
        updateModTimes();
    }

    /// <summary>
    /// Run the shader program.
    /// </summary>
    void use() const { glUseProgram(ID); }

    // uniform setters
    void setBool(const std::string& name, bool value) const {
        glUniform1i(getLocation(name), value);
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(getLocation(name), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(getLocation(name), value);
    }
    void setVec2(const std::string& name, const glm::vec2& v) const {
        glUniform2fv(getLocation(name), 1, &v[0]);
    }
    void setVec2(const std::string& name, float x, float y) const {
        glUniform2f(getLocation(name), x, y);
    }
    void setVec3(const std::string& name, const glm::vec3& v) const {
        glUniform3fv(getLocation(name), 1, &v[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const {
        glUniform3f(getLocation(name), x, y, z);
    }
    void setVec4(const std::string& name, const glm::vec4& v) const {
        glUniform4fv(getLocation(name), 1, &v[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const {
        glUniform4f(getLocation(name), x, y, z, w);
    }
    void setMat2(const std::string& name, const glm::mat2& m) const {
        glUniformMatrix2fv(getLocation(name), 1, GL_FALSE, &m[0][0]);
    }
    void setMat3(const std::string& name, const glm::mat3& m) const {
        glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, &m[0][0]);
    }
    void setMat4(const std::string& name, const glm::mat4& m) const {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, &m[0][0]);
    }

    //explicit operator bool() const noexcept {
    //    return ID;
    //}

private:
    std::string m_vertexPath, m_fragmentPath, m_geometryPath;
    time_t m_vertexModTime = 0, m_fragmentModTime = 0, m_geometryModTime = 0;

    int getLocation(const std::string& name) const {
        return glGetUniformLocation(ID, name.c_str());
    }

    static time_t getModTime(const std::string& path) {
        struct stat st;
        return (stat(path.c_str(), &st) == 0) ? st.st_mtime : 0;
    }

    void updateModTimes() {
        m_vertexModTime = getModTime(m_vertexPath);
        m_fragmentModTime = getModTime(m_fragmentPath);
        m_geometryModTime = m_geometryPath.empty() ? 0 : getModTime(m_geometryPath);
    }

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "ERROR::SHADER_FILE_NOT_FOUND: " << path << "\n";
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    unsigned int compileShader(const std::string& source, GLenum type, const char* typeName) {
        unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR: " << typeName << "\n" << infoLog << "\n";
        }

        return shader;
    }

    void compile() {
        std::string vertexCode = readFile(m_vertexPath);
        std::string fragmentCode = readFile(m_fragmentPath);
        std::string geometryCode = m_geometryPath.empty() ? "" : readFile(m_geometryPath);

        unsigned int vertex = compileShader(vertexCode, GL_VERTEX_SHADER, "VERTEX");
        unsigned int fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER, "FRAGMENT");
        unsigned int geometry = 0;
        if (!m_geometryPath.empty()) {
            geometry = compileShader(geometryCode, GL_GEOMETRY_SHADER, "GEOMETRY");
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometry) glAttachShader(ID, geometry);
        glLinkProgram(ID);

        GLint success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(ID, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << "\n";
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometry) glDeleteShader(geometry);
    }
};