#pragma once

#include "../SFMLOGL.h"

#include "../../assetmanager/Asset.h"

#include <string>

namespace pge {
	class Texture2DArray : public Asset {
	private:
		GLuint textureID;

		sf::Vector2u size;
		unsigned int layers;

	public:
		// Inherited from the Asset
		bool createAsset(const std::string &name);

		void genMipMaps();

		void bind() const {
			glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
		}

		unsigned int getWidth() const {
			return size.x;
		}

		unsigned int getHeight() const {
			return size.y;
		}

		size_t getLayers() const {
			return layers;
		}

		GLuint getTextureID() const {
			return textureID;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new Texture2DArray();
		}
	};
}