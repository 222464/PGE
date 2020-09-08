#pragma once

#include "../../scene/SceneObject.h"

#include "SceneObjectLighting.h"

#include "../../constructs/Vec3f.h"

#include "../../util/Math.h"

namespace pge {
	class SceneObjectSpotLight : public SceneObject {
	private:
		SceneObjectRef lighting;

		Vec3f position;
		Vec3f color;

		float range;

		Vec3f direction;
		float spotAngle;
		float spotAngleCos;
		float endConeRadius;
		float exponent;

		Matrix4x4f transform;

		std::shared_ptr<VBO> uniformBuffer;

		bool needsUniformBufferUpdate;

	public:
		bool enabled;

		SceneObjectSpotLight();

		void create(SceneObjectLighting* pLighting);

		// Inherited from SceneObject
		void deferredRender();

		void setPosition(const Vec3f &position);
		void setColor(const Vec3f &color);
		void setRange(float range);
		void setDirection(const Vec3f &direction);
		void setSpotAngle(float angle);
		void setExponent(float exponent);

		const Vec3f &getPosition() const {
			return position;
		}

		const Vec3f &getColor() const {
			return color;
		}

		float getRange() const {
			return range;
		}

		const Vec3f &getDirection() const {
			return direction;
		}

		float getSpotAngle() const {
			return spotAngle;
		}

		float getSpotAngleCos() const {
			return spotAngleCos;
		}

		float getSpotAngleCosFlipInv() const {
			return 1.0f / (1.0f - spotAngleCos);
		}

		float getExponent() const {
			return exponent;
		}

		void updateUniformBuffer();

		void setTransform(RenderScene* pRenderScene) {
			pRenderScene->setTransform(transform);
		}

		void updateSpotLightInternals();

		SceneObject* copyFactory() {
			return new SceneObjectSpotLight(*this);
		}

		friend SceneObjectLighting;
	};
}