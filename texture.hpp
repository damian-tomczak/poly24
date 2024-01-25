#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <filesystem>

#include "glad/glad.h"
#include "stb_image.h"

class Texture final
{
    inline static const std::filesystem::path texturesPath{"assets/textures"};

    GLuint mId{};
    GLuint mInternalFormat{GL_RGB};
    GLuint mImageFormat{GL_RGB};
    GLuint mWrapS{GL_REPEAT};
    GLuint mWrapT{GL_REPEAT};
    GLuint mFilterMin{GL_LINEAR};
    GLuint mFilterMax{GL_LINEAR};
    int mWidth{}, mHeight{};

public:

    Texture(const std::filesystem::path& texturePath)
    {
        glGenTextures(1, &mId);

        stbi_set_flip_vertically_on_load(true);

        const auto fullPath = texturesPath / texturePath;
        int nrChannels;
        const auto data = stbi_load(fullPath.string().c_str(), &mWidth, &mHeight, &nrChannels, 0);
        assert(data);

        glBindTexture(GL_TEXTURE_2D, mId);
        glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mImageFormat, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilterMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilterMax);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind() const
    {
        glBindTexture(GL_TEXTURE_2D, mId);
    }
};