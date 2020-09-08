#pragma once

#include "../../assetmanager/Asset.h"
#include <SFML/Audio.hpp>

namespace pge {
	class SoundBuffer : public Asset {
	private:
		sf::SoundBuffer buffer;
		
	public:
		void setBuffer(sf::Sound &sound) {
			sound.setBuffer(buffer);
		}

		// Inherited from Asset
		bool createAsset(const std::string &name);

		// Asset factory
		static Asset* assetFactory() {
			return new SoundBuffer();
		}

		friend class SceneObjectSoundSource;
	};
}