#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/lighting/SceneObjectLighting.h>

#include <pge/rendering/bufferobjects/CubeMapDepthRT.h>

#include <pge/constructs/Vec3f.h>

namespace pge {
	class SceneObjectPointLightShadowed : public SceneObject {
	private:
		SceneObjectRef _lighting;

		Vec3f _position;
		Vec3f _color;

		float _range;

		std::shared_ptr<VBO> _uniformBuffer;

		std::shared_ptr<CubeMapDepthRT> _cubeMap;

		std::shared_ptr<std::array<std::vector<AABB3D>, 6>> _faceObjects;

		bool _needsUniformBufferUpdate;

	public:
		bool _enabled;
		
		std::array<bool, 6> _updateFaces;

		float _shadowMapZNear;

		SceneObjectPointLightShadowed();

		void create(SceneObjectLighting* pLighting, unsigned int size);

		// Inherited from SceneObject
		void preRender();
		void deferredRender();

		void setPosition(const Vec3f &position);
		void setColor(const Vec3f &color);
		void setRange(float range);

		const Vec3f &getPosition() const {
			return _position;
		}

		const Vec3f &getColor() const {
			return _color;
		}

		float getRange() const {
			return _range;
		}

		void updateUniformBuffer();

		void setTransform(RenderScene* pRenderScene) {
			pRenderScene->setTransform(Matrix4x4f::translateMatrix(_position) * Matrix4x4f::scaleMatrix(Vec3f(_range, _range, _range)));
		}

		SceneObject* copyFactory() {
			return new SceneObjectPointLightShadowed(*this);
		}

		void setShadowMap(Shader* pShader);

		friend SceneObjectLighting;
	};
}