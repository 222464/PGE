#pragma once

#include "../SFMLOGL.h"

#include "../../assetmanager/Asset.h"

#include <string>

#include <memory>

namespace pge {
	class Texture2D : public Asset {
	private:
		GLuint textureID;

		sf::Texture texture;

		std::unique_ptr<sf::Image> image;

	public:
		// Inherited from the Asset
		bool createAsset(const std::string &name);

		void genMipMaps();

		void discardSoftwareImage() {
			image.reset();
		}

		sf::Image* getImage() {
			if (image == nullptr)
				image.reset(new sf::Image(texture.copyToImage()));
			
			return image.get();
		}

		void bind() const {
			glBindTexture(GL_TEXTURE_2D, textureID);
		}

		unsigned int getWidth() const {
			return texture.getSize().x;
		}

		unsigned int getHeight() const {
			return texture.getSize().y;
		}

		GLuint getTextureID() const {
			return textureID;
		}

		const sf::Texture &getTexture() const {
			return texture;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new Texture2D();
		}
	};
}
