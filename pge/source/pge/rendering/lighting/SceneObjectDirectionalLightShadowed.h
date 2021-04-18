#pragma once

#include "../../scene/SceneObject.h"

#include "SceneObjectLighting.h"
#include "../bufferobjects/DepthRT.h"

#include "../../constructs/Vec3f.h"

namespace pge {
    class SceneObjectDirectionalLightShadowed : public SceneObject {
    private:
        SceneObjectRef lighting;

        Vec3f color;
        Vec3f direction;

        std::shared_ptr<VBO> uniformBuffer;

        std::vector<Vec4f> splitDistances;
        std::vector<Matrix4x4f> lightBiasViewProjections;
        std::vector<std::shared_ptr<DepthRT>> cascades;

        bool needsUniformBufferUpdate;

        float zNear, zFar;

        void getFrustumCornerPoints(float zDistance, std::array<Vec3f, 4> &points);

    public:
        bool enabled;

        float downwardsRangeExtension;
        float upwardsRangeExtension;

        float sidewaysRangeExtensionMultiplier;

        SceneObjectDirectionalLightShadowed();

        void create(SceneObjectLighting* pLighting, int numCascades, unsigned int cascadeResolution, float zNear, float zFar, float gamma);

        // Inherited from SceneObject
        void preRender();
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
            return new SceneObjectDirectionalLightShadowed(*this);
        }

        void setCascadeShadowMaps(Shader* pShader);

        friend SceneObjectLighting;
    };
}
