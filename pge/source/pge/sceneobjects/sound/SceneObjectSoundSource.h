#pragma once

#include "../../scene/SceneObject.h"
#include "SoundBuffer.h"

namespace pge {
	class SceneObjectSoundSource : public SceneObject {
	private:
		sf::Sound source;

	public:
		void setBuffer(SoundBuffer* pBuffer) {
			source.setBuffer(pBuffer->buffer);
		}

		void setPosition(const Vec3f &position) {
			source.setPosition(sf::Vector3f(position.x, position.y, position.z));
		}

		Vec3f getPosition() const {
			return Vec3f(source.getPosition().x, source.getPosition().y, source.getPosition().z);
		}

		void setLooping(bool looping) {
			source.setLoop(looping);
		}

		bool isLooping() const {
			return source.getLoop();
		}

		void setVolume(float volume) {
			source.setVolume(volume);
		}

		float getVolume() const {
			return source.getVolume();
		}

		void setPitch(float pitch) {
			source.setPitch(pitch);
		}

		float getPitch() const {
			return source.getPitch();
		}

		void play() {
			source.play();
		}

		void pause() {
			source.pause();
		}

		void stop() {
			source.stop();
		}

		void setPlayingOffset(float time) {
			source.setPlayingOffset(sf::seconds(time));
		}

		float getPlayingOffset() const {
			return source.getPlayingOffset().asSeconds();
		}
	};
}