#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/lighting/SceneObjectLighting.h>

#include <pge/constructs/Vec3f.h>

namespace pge {
	class SceneObjectDirectionalLight : public SceneObject {
	private:
		SceneObjectRef _lighting;

		Vec3f _color;
		Vec3f _direction;

		std::shared_ptr<VBO> _uniformBuffer;

		bool _needsUniformBufferUpdate;

	public:
		bool _enabled;

		SceneObjectDirectionalLight();

		void create(SceneObjectLighting* pLighting);

		// Inherited from SceneObject
		void deferredRender();

		void setColor(const Vec3f &color);
		void setDirection(const Vec3f &direction);

		const Vec3f &getColor() const {
			return _color;
		}

		const Vec3f getDirection() const {
			return _direction;
		}

		void updateUniformBuffer();

		SceneObject* copyFactory() {
			return new SceneObjectDirectionalLight(*this);
		}

		friend SceneObjectLighting;
	};
}