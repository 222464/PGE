#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

using namespace pge;

void SceneObjectPhysicsWorld::onQueue() {
	_pBroadphase.reset(new btDbvtBroadphase());
	_pCollisionConfiguration.reset(new btDefaultCollisionConfiguration());
	_pDispatcher.reset(new btCollisionDispatcher(_pCollisionConfiguration.get()));

	_pSolver.reset(new btMultiBodyConstraintSolver());

	_pDynamicsWorld.reset(new btMultiBodyDynamicsWorld(_pDispatcher.get(), _pBroadphase.get(), _pSolver.get(), _pCollisionConfiguration.get()));

	_pGhostPairCallBack.reset(new btGhostPairCallback());

	_pDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

	_pDynamicsWorld->getPairCache()->setInternalGhostPairCallback(_pGhostPairCallBack.get());
}

void SceneObjectPhysicsWorld::onDestroy() {
	_pDynamicsWorld.reset();

	_pGhostPairCallBack.reset();

	_pSolver.reset();

	_pDispatcher.reset();

	_pCollisionConfiguration.reset();

	_pBroadphase.reset();
}

void SceneObjectPhysicsWorld::synchronousUpdate(float dt) {
	assert(_steps > 0);

	float timeStep = dt / _steps;

	for (int i = 0; i < _steps; i++)
		_pDynamicsWorld->stepSimulation(timeStep, _subSteps);
}