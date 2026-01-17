#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <stdexcept>

#include <logger.h>


// shader exception type
class ShaderException : public std::runtime_error {
public:
    explicit ShaderException(const std::string& message) : std::runtime_error(message) {}
};

/// <summary>
/// This class encapsulates an OpenGL shader program.
/// </summary>
class Shader {
public:
    unsigned int ID = 0;

    Shader() = default;
    Shader(const char* vertexPath, const char* fragmentPath) : m_vertexPath(vertexPath), m_fragmentPath(fragmentPath) {
        if (!compile()) {
            throw ShaderException("Initial shader compilation failed.");
        }
        updateModTimes();
    }
    ~Shader() {
        if (ID != 0 && glIsProgram(ID)) {
            glDeleteProgram(ID);
        }
    }

    /// <summary>
    /// Run the shader program.
    /// </summary>
    void use() const {
        if (ID == 0 || !glIsProgram(ID)) {
            throw ShaderException("Attempted to use invalid shader program.");
        }
        glUseProgram(ID);
    }

    /// <summary>
    /// Checks shader source files for modification and recompiles if needed
    /// </summary>
    /// <returns>True if a reload occured and succeeded</returns>
    bool checkHotReload() {
        time_t vMod = getModTime(m_vertexPath);
        time_t fMod = getModTime(m_fragmentPath);

        if (vMod == 0 || fMod == 0) return false;

        if (vMod != m_vertexModTime || fMod != m_fragmentModTime) {
            m_vertexModTime = vMod;
            m_fragmentModTime = fMod;

            if (!compile()) {
                logger.error("Shader hot reload failed");
                return false;
            }

            logger.info(std::to_string(ID) + " shader reloaded");
            return true;
        }

        return false;
    }

    // uniform setters
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec2(const std::string& name, const glm::vec2& v) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &v[0]);
    }
    void setVec2(const std::string& name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setVec3(const std::string& name, const glm::vec3& v) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &v[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setVec4(const std::string& name, const glm::vec4& v) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &v[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    void setMat2(const std::string& name, const glm::mat2& m) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &m[0][0]);
    }
    void setMat3(const std::string& name, const glm::mat3& m) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &m[0][0]);
    }
    void setMat4(const std::string& name, const glm::mat4& m) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &m[0][0]);
    }

private:
    std::string m_vertexPath;
    std::string m_fragmentPath;

    time_t m_vertexModTime = 0;
    time_t m_fragmentModTime = 0;

    // get file modification time
    static time_t getModTime(const std::string& path) {
        struct stat st;
        return (stat(path.c_str(), &st) == 0) ? st.st_mtime : 0;
    }

    // store current modification times
    void updateModTimes() {
        m_vertexModTime = getModTime(m_vertexPath);
        m_fragmentModTime = getModTime(m_fragmentPath);
    }

    // read shader source code from file
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            logger.error("SHADER_FILE_NOT_FOUND " + path);
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // compile a single shader and check for errors
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
            logger.error(std::string("SHADER_COMPILATION_ERROR: ") + typeName + "\n" + infoLog);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    // load, compile, and link the shader program
    bool compile() {
        std::string vertexCode = readFile(m_vertexPath);
        std::string fragmentCode = readFile(m_fragmentPath);

        unsigned int vertex = compileShader(vertexCode, GL_VERTEX_SHADER, "VERTEX");
        if (vertex == 0) return false;
        unsigned int fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER, "FRAGMENT");
        if (fragment == 0) {
            glDeleteShader(vertex);
            return false;
        }

        unsigned int program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(program, 1024, nullptr, infoLog);
            logger.error("PROGRAM_LINKING_ERROR\n" + std::string(infoLog));
            glDeleteProgram(program);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return false;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        ID = program;
        return true;
    }
};