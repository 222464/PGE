#pragma once

#include <pge/assetmanager/Asset.h>

#include <SFML/Graphics.hpp>

class Font : public pge::Asset {
private:
public:
	sf::Font _font;

	// Inherited from Asset
	bool createAsset(const std::string &name);

	// Asset factory
	static Asset* assetFactory() {
		return new Font();
	}
};