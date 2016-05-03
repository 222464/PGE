#include <pge/bvh/BVHNode.h>

#include <pge/bvh/BVHTree.h>

using namespace pge;

const float BVHNode::_traverseCost = 8.0f;
const float BVHNode::_intersectCost = 1.0f;

BVHNode::BVHNode(class BVHTree* pTree, BVHNode* pParent)
: _pTree(pTree), _pParent(pParent), _numOccupantsBelow(0), _collapseMarker(false), _pLeft(nullptr), _pRight(nullptr)
{}

void BVHNode::add(const FormTriangle &triangle, const AABB3D &triangleAABB) {
	_numOccupantsBelow++;

    // Add here
    _occupants.push_back(triangle);
}

float BVHNode::getCostSAH(const Vec3f &splitPos, int axis) {
	Vec3f dims = _aabb.getDims();
	float a = 2.0f * (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z);
	float aInv = 1.0f / a;

	switch (axis) {
	case 0:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(splitPos.x, _aabb._upperBound.y, _aabb._upperBound.z));
			AABB3D aabb1(Vec3f(splitPos.x, _aabb._lowerBound.y, _aabb._lowerBound.z), _aabb._upperBound);

			Vec3f dims0 = aabb0.getDims();
			Vec3f dims1 = aabb1.getDims();

			float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
			float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

			float n0 = 0.0f;
			float n1 = 0.0f;

			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (aabb0.intersects(aabb))
					n0++;
				if (aabb1.intersects(aabb))
					n1++;
			}

			return _traverseCost + _intersectCost * (a0 * n0 + a1 * n1) * aInv;
		}

	case 1:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, splitPos.y, _aabb._upperBound.z));
			AABB3D aabb1(Vec3f(_aabb._lowerBound.x, splitPos.y, _aabb._lowerBound.z), _aabb._upperBound);

			Vec3f dims0 = aabb0.getDims();
			Vec3f dims1 = aabb1.getDims();

			float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
			float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

			float n0 = 0.0f;
			float n1 = 0.0f;

			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (aabb0.intersects(aabb))
					n0++;
				if (aabb1.intersects(aabb))
					n1++;
			}

			return _traverseCost + _intersectCost * (a0 * n0 + a1 * n1) * aInv;
		}

	case 2:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, _aabb._upperBound.y, splitPos.z));
			AABB3D aabb1(Vec3f(_aabb._lowerBound.x, _aabb._lowerBound.y, splitPos.z), _aabb._upperBound);

			Vec3f dims0 = aabb0.getDims();
			Vec3f dims1 = aabb1.getDims();

			float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
			float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

			float n0 = 0.0f;
			float n1 = 0.0f;

			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (aabb0.intersects(aabb))
					n0++;
				if (aabb1.intersects(aabb))
					n1++;
			}

			return _traverseCost + _intersectCost * (a0 * n0 + a1 * n1) * aInv;
		}
	}

	return 0.0f;
}

// Returns if should split 
bool BVHNode::findSplitPosSAH(float &pos, int &axis, size_t &splitIndex) {
	// Get sorted arrays in all directions
	// Get min and max for all axes
	std::vector<CoordAndIsMin> sortedX(_occupants.size() * 2);
	std::vector<CoordAndIsMin> sortedY(_occupants.size() * 2);
	std::vector<CoordAndIsMin> sortedZ(_occupants.size() * 2);

	for (size_t i = 0; i < _occupants.size(); i++) {
		AABB3D aabb = _occupants[i].getAABB();

		sortedX[i * 2]._coord = aabb._lowerBound.x;
		sortedX[i * 2]._isMin = true;
		sortedX[i * 2 + 1]._coord = aabb._upperBound.x;
		sortedX[i * 2 + 1]._isMin = false;

		sortedY[i * 2]._coord = aabb._lowerBound.y;
		sortedY[i * 2]._isMin = true;
		sortedY[i * 2 + 1]._coord = aabb._upperBound.y;
		sortedY[i * 2 + 1]._isMin = false;

		sortedZ[i * 2]._coord = aabb._lowerBound.z;
		sortedZ[i * 2]._isMin = true;
		sortedZ[i * 2 + 1]._coord = aabb._upperBound.z;
		sortedZ[i * 2 + 1]._isMin = false;
	}

	std::sort(sortedX.begin(), sortedX.end());
	std::sort(sortedY.begin(), sortedY.end());
	std::sort(sortedZ.begin(), sortedZ.end());

	float currentCost = _intersectCost * _occupants.size();

	float bestCost = 999999.0f;

	// X axis
	int tL = 1;
	int tR = _occupants.size();

	bool maxEvent = false;

	Vec3f dims = _aabb.getDims();

	float aInv = 0.5f / (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z);

	for (size_t i = 0; i < sortedX.size(); i++) {
		if (sortedX[i]._coord <= _aabb._lowerBound.x || sortedX[i]._coord >= _aabb._upperBound.x)
			continue;

		AABB3D aabbL(_aabb._lowerBound, Vec3f(sortedX[i]._coord, _aabb._upperBound.y, _aabb._upperBound.z));
		AABB3D aabbR(Vec3f(sortedX[i]._coord, _aabb._lowerBound.y, _aabb._lowerBound.z), _aabb._upperBound);

		Vec3f dimsL = aabbL.getDims();
		Vec3f dimsR = aabbR.getDims();

		float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
		float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

		float cost = _traverseCost + _intersectCost * (aL * tL + aR * tR) * aInv;

		if (cost < bestCost) {
			bestCost = cost;
			pos = sortedX[i]._coord;
			axis = 0;
			splitIndex = i;
		}

		if (maxEvent) {
			tR--;
			maxEvent = false;
		}

		if (sortedX[i]._isMin)
			tL++;
		else
			maxEvent = true;
	}

	tL = 1;
	tR = _occupants.size();

	maxEvent = false;

	for (size_t i = 0; i < sortedY.size(); i++) {
		if (sortedY[i]._coord <= _aabb._lowerBound.y || sortedY[i]._coord >= _aabb._upperBound.y)
			continue;

		AABB3D aabbL(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, sortedY[i]._coord, _aabb._upperBound.z));
		AABB3D aabbR(Vec3f(_aabb._lowerBound.x, sortedY[i]._coord, _aabb._lowerBound.z), _aabb._upperBound);

		Vec3f dimsL = aabbL.getDims();
		Vec3f dimsR = aabbR.getDims();

		float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
		float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

		float cost = _traverseCost + _intersectCost * (aL * tL + aR * tR) * aInv;

		if (cost < bestCost) {
			bestCost = cost;
			pos = sortedY[i]._coord;
			axis = 1;
			splitIndex = i;
		}

		if (maxEvent) {
			tR--;
			maxEvent = false;
		}

		if (sortedY[i]._isMin)
			tL++;
		else
			maxEvent = true;
	}

	tL = 1;
	tR = _occupants.size();

	maxEvent = false;

	for (size_t i = 0; i < sortedZ.size(); i++) {
		if (sortedZ[i]._coord <= _aabb._lowerBound.z || sortedZ[i]._coord >= _aabb._upperBound.z)
			continue;

		AABB3D aabbL(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, _aabb._upperBound.y, sortedZ[i]._coord));
		AABB3D aabbR(Vec3f(_aabb._lowerBound.x, _aabb._lowerBound.y, sortedZ[i]._coord), _aabb._upperBound);

		Vec3f dimsL = aabbL.getDims();
		Vec3f dimsR = aabbR.getDims();

		float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
		float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

		float cost = _traverseCost + _intersectCost * (aL * tL + aR * tR) * aInv;

		if (cost < bestCost) {
			bestCost = cost;
			pos = sortedZ[i]._coord;
			axis = 2;
			splitIndex = i;
		}

		if (maxEvent) {
			tR--;
			maxEvent = false;
		}

		if (sortedZ[i]._isMin)
			tL++;
		else
			maxEvent = true;
	}

	if (currentCost <= bestCost)
		return false;

	return true;
}

void BVHNode::split(int numSplitsAfterNoTriangleReduction) {
	float splitPos;
	int axis;
	size_t splitIndex;

	if (!findSplitPosSAH(splitPos, axis, splitIndex))
		return;

	/*if (_aabb.GetHalfDims().x > _aabb.GetHalfDims().y)
	{
		if (_aabb.GetHalfDims().x > _aabb.GetHalfDims().z)
		{
			axis = 0;
			splitPos = _aabb.GetCenter().x;
		}
		else
		{
			axis = 2;
			splitPos = _aabb.GetCenter().z;
		}
	}
	else
	{
		if (_aabb.GetHalfDims().y > _aabb.GetHalfDims().z)
		{
			axis = 1;
			splitPos = _aabb.GetCenter().y;
		}
		else
		{
			axis = 2;
			splitPos = _aabb.GetCenter().z;
		}
	}*/

	_pLeft.reset(new BVHNode(_pTree, this));
	_pRight.reset(new BVHNode(_pTree, this));

	switch (axis)
	{
	case 0:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(splitPos, _aabb._upperBound.y, _aabb._upperBound.z));
			AABB3D aabb1(Vec3f(splitPos, _aabb._lowerBound.y, _aabb._lowerBound.z), _aabb._upperBound);

			_pLeft->_aabb = aabb0;
			_pRight->_aabb = aabb1;

			// Add occupants to the new nodes if they fit
			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (_pLeft->_aabb.intersects(aabb))
					_pLeft->add(_occupants[i], aabb);
				if (_pRight->_aabb.intersects(aabb))
					_pRight->add(_occupants[i], aabb);
			}

			if (numSplitsAfterNoTriangleReduction < _pTree->_maxSplitsAfterNoTriangleReduction) {
				// Left split
				_pLeft->split(_pLeft->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

				// Right split
				_pRight->split(_pRight->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
			}

			break;
		}
	case 1:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, splitPos, _aabb._upperBound.z));
			AABB3D aabb1(Vec3f(_aabb._lowerBound.x, splitPos, _aabb._lowerBound.z), _aabb._upperBound);

			_pLeft->_aabb = aabb0;
			_pRight->_aabb = aabb1;

			// Add occupants to the new nodes if they fit
			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (_pLeft->_aabb.intersects(aabb))
					_pLeft->add(_occupants[i], aabb);
				if (_pRight->_aabb.intersects(aabb))
					_pRight->add(_occupants[i], aabb);
			}

			if (numSplitsAfterNoTriangleReduction < _pTree->_maxSplitsAfterNoTriangleReduction) {
				// Left split
				_pLeft->split(_pLeft->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

				// Right split
				_pRight->split(_pRight->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
			}

			break;
		}
	case 2:
		{
			AABB3D aabb0(_aabb._lowerBound, Vec3f(_aabb._upperBound.x, _aabb._upperBound.y, splitPos));
			AABB3D aabb1(Vec3f(_aabb._lowerBound.x, _aabb._lowerBound.y, splitPos), _aabb._upperBound);

			_pLeft->_aabb = aabb0;
			_pRight->_aabb = aabb1;

			// Add occupants to the new nodes if they fit
			for (size_t i = 0, size = _occupants.size(); i < size; i++) {
				AABB3D aabb = _occupants[i].getAABB();

				if (_pLeft->_aabb.intersects(aabb))
					_pLeft->add(_occupants[i], aabb);
				if (_pRight->_aabb.intersects(aabb))
					_pRight->add(_occupants[i], aabb);
			}

			if (numSplitsAfterNoTriangleReduction < _pTree->_maxSplitsAfterNoTriangleReduction) {
				// Left split
				_pLeft->split(_pLeft->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

				// Right split
				_pRight->split(_pRight->_occupants.size() == _occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
			}

			break;
		}
	}

	_occupants.clear();
}