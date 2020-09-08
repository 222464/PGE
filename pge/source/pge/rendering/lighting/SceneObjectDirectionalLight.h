#pragma once

#include "../../scene/SceneObject.h"

#include "SceneObjectLighting.h"

#include "../../constructs/Vec3f.h"

namespace pge {
	class SceneObjectDirectionalLight : public SceneObject {
	private:
		SceneObjectRef lighting;

		Vec3f color;
		Vec3f direction;

		std::shared_ptr<VBO> uniformBuffer;

		bool needsUniformBufferUpdate;

	public:
		bool enabled;

		SceneObjectDirectionalLight();

		void create(SceneObjectLighting* pLighting);

		// Inherited from SceneObject
		void deferredRender();

		void setColor(const Vec3f &color);
		void setDirection(const Vec3f &direction);

		const Vec3f &getColor() const {
			return color;
		}

		const Vec3f getDirection() const {
			return direction;
		}

		void updateUniformBuffer();

		SceneObject* copyFactory() {
			return new SceneObjectDirectionalLight(*this);
		}

		friend SceneObjectLighting;
	};
}