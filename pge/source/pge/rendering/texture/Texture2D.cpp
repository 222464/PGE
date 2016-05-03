#include <pge/rendering/texture/Texture2D.h>

#include <iostream>
#include <sstream>

#include <assert.h>

using namespace pge;

bool Texture2D::createAsset(const std::string &name) {
	std::istringstream is(name);

	std::string fileName;

	is >> fileName;

	_image.reset(new sf::Image());

	_image->loadFromFile(fileName);

	_texture.loadFromImage(*_image);

	if (is.eof() || !is.good())
		discardSoftwareImage(); 
	else {
		std::string shouldKeepImage;

		is >> shouldKeepImage;

		if (shouldKeepImage != "true")
			discardSoftwareImage();
	}

	bind();

	GLint textureID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureID);
	_textureID = static_cast<GLuint>(textureID);

	// Default settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	PGE_GL_ERROR_CHECK();

	return true;
}

void Texture2D::genMipMaps() {
	bind();

	// Only min filter, since mipmapping is only for when zoomed out a lot
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}