#include "TextureCube.h"

#include <iostream>

#include <sstream>

#include <assert.h>

using namespace pge;

bool TextureCube::createAsset(const std::string &name) {

    std::istringstream fromString(name);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (size_t i = 0; i < 6; i++) {
        std::string subName;

        fromString >> subName;

        sf::Image img;

        if (!img.loadFromFile(subName))
            return false;

        size = img.getSize().x;

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    }

    // Default settings
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    PGE_GL_ERROR_CHECK();

    return true;
}

void TextureCube::genMipMaps() {
    bind();

    // Only min filter, since mipmapping is only for when zoomed out a lot
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}