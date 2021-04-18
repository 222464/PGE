#pragma once

#include "../SFMLOGL.h"

#include "../../assetmanager/Asset.h"

#include <array>

#include <string>

namespace pge {
    class TextureCube : public Asset {
    private:
        GLuint textureID;

        unsigned int size;

    public:
        // Inherited from the Asset
        bool createAsset(const std::string &name);

        void genMipMaps();

        void bind() const {
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        }

        unsigned int getSize() const {
            return size;
        }

        GLuint getTextureID() const {
            return textureID;
        }

        // Asset factory
        static Asset* assetFactory() {
            return new TextureCube();
        }
    };
}