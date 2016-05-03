#pragma once

#include <pge/scene/SceneObject.h>
#include <pge/sceneobjects/sound/SoundBuffer.h>

namespace pge {
	class SceneObjectSoundSource : public SceneObject {
	private:
		sf::Sound _source;

	public:
		void setBuffer(SoundBuffer* pBuffer) {
			_source.setBuffer(pBuffer->_buffer);
		}

		void setPosition(const Vec3f &position) {
			_source.setPosition(sf::Vector3f(position.x, position.y, position.z));
		}

		Vec3f getPosition() const {
			return Vec3f(_source.getPosition().x, _source.getPosition().y, _source.getPosition().z);
		}

		void setLooping(bool looping) {
			_source.setLoop(looping);
		}

		bool isLooping() const {
			return _source.getLoop();
		}

		void setVolume(float volume) {
			_source.setVolume(volume);
		}

		float getVolume() const {
			return _source.getVolume();
		}

		void setPitch(float pitch) {
			_source.setPitch(pitch);
		}

		float getPitch() const {
			return _source.getPitch();
		}

		void play() {
			_source.play();
		}

		void pause() {
			_source.pause();
		}

		void stop() {
			_source.stop();
		}

		void setPlayingOffset(float time) {
			_source.setPlayingOffset(sf::seconds(time));
		}

		float getPlayingOffset() const {
			return _source.getPlayingOffset().asSeconds();
		}
	};
}