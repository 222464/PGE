#pragma once

#include <pge/rendering/SFMLOGL.h>

#include <pge/assetmanager/Asset.h>

#include <string>

#include <memory>

namespace pge {
	class Texture2D : public Asset {
	private:
		GLuint _textureID;

		sf::Texture _texture;

		std::unique_ptr<sf::Image> _image;

	public:
		// Inherited from the Asset
		bool createAsset(const std::string &name);

		void genMipMaps();

		void discardSoftwareImage() {
			_image.reset();
		}

		sf::Image* getImage() {
			if (_image == nullptr)
				_image.reset(new sf::Image(_texture.copyToImage()));
			
			return _image.get();
		}

		void bind() const {
			sf::Texture::bind(&_texture);
		}

		unsigned int getWidth() const {
			return _texture.getSize().x;
		}

		unsigned int getHeight() const {
			return _texture.getSize().y;
		}

		GLuint getTextureID() const {
			return _textureID;
		}

		const sf::Texture &getTexture() const {
			return _texture;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new Texture2D();
		}
	};
}