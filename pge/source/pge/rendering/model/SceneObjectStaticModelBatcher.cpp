#include "SceneObjectStaticModelBatcher.h"

using namespace pge;

void SceneObjectStaticModelBatcher::batchRender() {
	for (std::unordered_map<StaticModel*, std::list<Matrix4x4f>>::iterator it0 = modelTransforms.begin(); it0 != modelTransforms.end(); it0++) {
		for (size_t i = 0; i < it0->first->meshes.size(); i++) {
			getRenderScene()->useShader(it0->first->materials[it0->first->meshes[i].materialIndex]);

			it0->first->meshes[i].mesh->setAttributes();

			for (std::list<Matrix4x4f>::iterator it1 = it0->second.begin(); it1 != it0->second.end(); it1++) {
				getRenderScene()->setTransform(*it1);

				it0->first->meshes[i].mesh->renderFromAttributes();
			}
		}
	}

	modelTransforms.clear();
}