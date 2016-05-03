#pragma once

#include <pge/rendering/SFMLOGL.h>

#include <pge/assetmanager/Asset.h>

#include <string>

namespace pge {
	class Texture2DArray : public Asset {
	private:
		GLuint _textureID;

		sf::Vector2u _size;
		unsigned int _layers;

	public:
		// Inherited from the Asset
		bool createAsset(const std::string &name);

		void genMipMaps();

		void bind() const {
			glBindTexture(GL_TEXTURE_2D_ARRAY, _textureID);
		}

		unsigned int getWidth() const {
			return _size.x;
		}

		unsigned int getHeight() const {
			return _size.y;
		}

		size_t getLayers() const {
			return _layers;
		}

		GLuint getTextureID() const {
			return _textureID;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new Texture2DArray();
		}
	};
}