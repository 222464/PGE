#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/lighting/SceneObjectLighting.h>
#include <pge/rendering/bufferobjects/DepthRT.h>

#include <pge/constructs/Vec3f.h>

namespace pge {
	class SceneObjectDirectionalLightShadowed : public SceneObject {
	private:
		SceneObjectRef _lighting;

		Vec3f _color;
		Vec3f _direction;

		std::shared_ptr<VBO> _uniformBuffer;

		std::vector<float> _splitDistances;
		std::vector<Matrix4x4f> _lightBiasViewProjections;
		std::vector<std::shared_ptr<DepthRT>> _cascades;

		bool _needsUniformBufferUpdate;

		float _zNear, _zFar;

		void getFrustumCornerPoints(float zDistance, std::array<Vec3f, 4> &points);

	public:
		bool _enabled;

		float _downwardsRangeExtension;
		float _upwardsRangeExtension;

		float _sidewaysRangeExtensionMultiplier;

		SceneObjectDirectionalLightShadowed();

		void create(SceneObjectLighting* pLighting, int numCascades, unsigned int cascadeResolution, float zNear, float zFar, float lambda);

		// Inherited from SceneObject
		void preRender();
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
			return new SceneObjectDirectionalLightShadowed(*this);
		}

		void setCascadeShadowMaps(Shader* pShader);

		friend SceneObjectLighting;
	};
}