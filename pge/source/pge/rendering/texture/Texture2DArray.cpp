#include <pge/rendering/texture/Texture2DArray.h>

#include <sstream>
#include <iostream>

using namespace pge;

bool Texture2DArray::createAsset(const std::string &name) {
	// Get names
	std::vector<std::string> names;

	std::istringstream iss(name);

	while (!iss.eof() && iss.good()) {
		std::string subName;
		iss >> subName;

		if (subName == "")
			break;

		names.push_back(subName);
	}

	if (names.empty()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not find any texture names in " << name << std::endl;
#endif
		return false;
	}

	_layers = names.size();

	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textureID);

	for (size_t i = 0; i < _layers; i++) {
		sf::Image image;

		if (!image.loadFromFile(names[i])) {
#ifdef PGE_DEBUG
			std::cerr << "Could not load image file " << names[i] << std::endl;
#endif
			return false;
		}

		// First texture determines format of others
		if (i == 0) {
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, image.getSize().x, image.getSize().y, _layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			_size = image.getSize();
		}
		else {
			if (image.getSize() != _size) {
#ifdef PGE_DEBUG
				std::cerr << "Image " << names[i] << " doesn't match the dimensions of the array!" << std::endl;
#endif
				return false;
			}
		}

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image.getSize().x, image.getSize().y, 1, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	}

	// Defaults
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	PGE_GL_ERROR_CHECK();

	return true;
}

void Texture2DArray::genMipMaps() {
	bind();

	// Only min filter, since mipmapping is only for when zoomed out a lot
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}