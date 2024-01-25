#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace std::string_literals;
namespace fs = std::filesystem;

enum class CompilationStage
{
    DEFAULT,
    VERTEX,
    FRAGMENT,
    PROGRAM,
    COUNT
};

std::ostream& operator<<(std::ostream& ostream, const CompilationStage stage)
{
    switch (stage)
    {
    case CompilationStage::VERTEX: ostream << "VERTEX"; break;
    case CompilationStage::FRAGMENT: ostream << "FRAGMENT"; break;
    case CompilationStage::PROGRAM: ostream << "PROGRAM"; break;
    default: ostream << "COMPILATION_STAGE_UNKNOWN"; break;
    }

    return ostream;
}

class Shader final
{
    inline static const fs::path shadersPath{"assets/shaders"};
    inline static bool isCompilationSuccess{true};

public:
    [[nodiscard]] static bool isShaderCompilationsSuccess() { return isCompilationSuccess; }

    GLuint mId;

    Shader(const fs::path& vertexPath, const fs::path& fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        const auto fullVertexPath = shadersPath / vertexPath;
        const auto fullFragmentPath = shadersPath / fragmentPath;

        assert(fs::exists(fullVertexPath));

        assert(fs::exists(fullFragmentPath));

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            vShaderFile.open(fullVertexPath);
            fShaderFile.open(fullFragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (const std::ifstream::failure& e)
        {
            throw std::runtime_error{"ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: "s + e.what()};
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        GLuint vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompilationErrors(vertex, CompilationStage::VERTEX, fullVertexPath);

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompilationErrors(fragment, CompilationStage::FRAGMENT, fullFragmentPath);

        mId = glCreateProgram();
        glAttachShader(mId, vertex);
        glAttachShader(mId, fragment);
        glLinkProgram(mId);
        checkCompilationErrors(mId, CompilationStage::PROGRAM);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use()
    {
        glUseProgram(mId);
    }

    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(mId, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(mId, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(mId, name.c_str()), value);
    }

private:
    void checkCompilationErrors(const GLuint shader, const CompilationStage stage, const fs::path& shaderFilePath = "")
    {
        static constexpr int infoLogSize{1024};

        GLint success;
        char infoLog[infoLogSize];

        std::ostringstream messagePrefix;
        if (stage != CompilationStage::PROGRAM)
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, infoLogSize, NULL, infoLog);
                messagePrefix << "ERROR::SHADER_COMPILATION_ERROR " << stage << " " << shaderFilePath;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, infoLogSize, NULL, infoLog);
                messagePrefix << "ERROR::SHADER_COMPILATION_ERROR ";
            }
        }

        if ((!success))
        {
            if (isCompilationSuccess)
            {
                isCompilationSuccess = false;
            }

            std::cout << messagePrefix.str() << "\n" << infoLog << "\n -- --------------------------------------------------- -- \n";
        }

    }
};
