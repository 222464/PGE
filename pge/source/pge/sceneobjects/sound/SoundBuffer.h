#pragma once

#include <pge/assetmanager/Asset.h>
#include <SFML/Audio.hpp>

namespace pge {
	class SoundBuffer : public Asset {
	private:
		sf::SoundBuffer _buffer;
		
	public:
		void setBuffer(sf::Sound &sound) {
			sound.setBuffer(_buffer);
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