#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/lighting/SceneObjectLighting.h>

#include <pge/constructs/Vec3f.h>

#include <pge/util/Math.h>

namespace pge {
	class SceneObjectSpotLight : public SceneObject {
	private:
		SceneObjectRef _lighting;

		Vec3f _position;
		Vec3f _color;

		float _range;

		Vec3f _direction;
		float _spotAngle;
		float _spotAngleCos;
		float _endConeRadius;
		float _exponent;

		Matrix4x4f _transform;

		std::shared_ptr<VBO> _uniformBuffer;

		bool _needsUniformBufferUpdate;

	public:
		bool _enabled;

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
			return _position;
		}

		const Vec3f &getColor() const {
			return _color;
		}

		float getRange() const {
			return _range;
		}

		const Vec3f &getDirection() const {
			return _direction;
		}

		float getSpotAngle() const {
			return _spotAngle;
		}

		float getSpotAngleCos() const {
			return _spotAngleCos;
		}

		float getSpotAngleCosFlipInv() const {
			return 1.0f / (1.0f - _spotAngleCos);
		}

		float getExponent() const {
			return _exponent;
		}

		void updateUniformBuffer();

		void setTransform(RenderScene* pRenderScene) {
			pRenderScene->setTransform(_transform);
		}

		void updateSpotLightInternals();

		SceneObject* copyFactory() {
			return new SceneObjectSpotLight(*this);
		}

		friend SceneObjectLighting;
	};
}