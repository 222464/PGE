#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

using namespace pge;

void SceneObjectStaticModelBatcher::batchRender() {
	for (std::unordered_map<StaticModel*, std::list<Matrix4x4f>>::iterator it0 = _modelTransforms.begin(); it0 != _modelTransforms.end(); it0++) {
		for (size_t i = 0; i < it0->first->_meshes.size(); i++) {
			getRenderScene()->useShader(it0->first->_materials[it0->first->_meshes[i]._materialIndex]);

			it0->first->_meshes[i]._mesh->setAttributes();

			for (std::list<Matrix4x4f>::iterator it1 = it0->second.begin(); it1 != it0->second.end(); it1++) {
				getRenderScene()->setTransform(*it1);

				it0->first->_meshes[i]._mesh->renderFromAttributes();
			}
		}
	}

	_modelTransforms.clear();
}