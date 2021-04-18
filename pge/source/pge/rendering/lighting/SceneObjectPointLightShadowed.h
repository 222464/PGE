#pragma once

#include "../../scene/SceneObject.h"

#include "SceneObjectLighting.h"

#include "../bufferobjects/CubeMapDepthRT.h"

#include "../../constructs/Vec3f.h"

namespace pge {
    class SceneObjectPointLightShadowed : public SceneObject {
    private:
        SceneObjectRef lighting;

        Vec3f position;
        Vec3f color;

        float range;

        std::shared_ptr<VBO> uniformBuffer;

        std::shared_ptr<CubeMapDepthRT> cubeMap;

        std::shared_ptr<std::array<std::vector<AABB3D>, 6>> faceObjects;

        bool needsUniformBufferUpdate;

    public:
        bool enabled;
        
        std::array<bool, 6> updateFaces;

        float shadowMapZNear;

        SceneObjectPointLightShadowed();

        void create(SceneObjectLighting* pLighting, unsigned int size);

        // Inherited from SceneObject
        void preRender();
        void deferredRender();

        void setPosition(const Vec3f &position);
        void setColor(const Vec3f &color);
        void setRange(float range);

        const Vec3f &getPosition() const {
            return position;
        }

        const Vec3f &getColor() const {
            return color;
        }

        float getRange() const {
            return range;
        }

        void updateUniformBuffer();

        void setTransform(RenderScene* pRenderScene) {
            pRenderScene->setTransform(Matrix4x4f::translateMatrix(position) * Matrix4x4f::scaleMatrix(Vec3f(range, range, range)));
        }

        SceneObject* copyFactory() {
            return new SceneObjectPointLightShadowed(*this);
        }

        void setShadowMap(Shader* pShader);

        friend SceneObjectLighting;
    };
}