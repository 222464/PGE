#include "SceneObjectPhysicsWorld.h"

using namespace pge;

SceneObjectPhysicsWorld::SceneObjectPhysicsWorld()
    : steps(3), subSteps(80)
{}

void SceneObjectPhysicsWorld::onQueue() {
    pBroadphase.reset(new btDbvtBroadphase());
    pCollisionConfiguration.reset(new btDefaultCollisionConfiguration());
    pDispatcher.reset(new btCollisionDispatcher(pCollisionConfiguration.get()));

    pSolver.reset(new btNNCGConstraintSolver());

    pDynamicsWorld.reset(new btDiscreteDynamicsWorld(pDispatcher.get(), pBroadphase.get(), pSolver.get(), pCollisionConfiguration.get()));

    pGhostPairCallBack.reset(new btGhostPairCallback());
    
    pDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

    pDynamicsWorld->getPairCache()->setInternalGhostPairCallback(pGhostPairCallBack.get());
}

void SceneObjectPhysicsWorld::onDestroy() {
    pDynamicsWorld.reset();

    pGhostPairCallBack.reset();

    pSolver.reset();

    pDispatcher.reset();

    pCollisionConfiguration.reset();

    pBroadphase.reset();
}

void SceneObjectPhysicsWorld::synchronousUpdate(float dt) {
    assert(steps > 0);

    float timeStep = dt / steps;

    for (int i = 0; i < steps; i++)
        pDynamicsWorld->stepSimulation(timeStep, subSteps);
}