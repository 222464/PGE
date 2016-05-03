constant sampler_t unnormalizedClampedNearestSampler = CLK_NORMALIZED_COORDS_FALSE |
	CLK_ADDRESS_CLAMP_TO_EDGE |
	CLK_FILTER_NEAREST;

constant sampler_t normalizedClampedNearestSampler = CLK_NORMALIZED_COORDS_TRUE |
	CLK_ADDRESS_CLAMP_TO_EDGE |
	CLK_FILTER_NEAREST;
	
constant const float randSeed = 0.0f;

constant const int maxKDSteps = 100;
constant const int maxTriangles = 20;
constant const int maxLights = 10;

constant const float3 attenuation = (float3)(0.1f, 0.001f, 0.00001f);

constant const int numAdditionalBounces = 1;
constant const int numPathSamples = 1;
constant const int numShadowSamples = 1;

constant const float testOffset = 0.01f;

constant const float lightJitterDist = 0.0f;

constant const float queryOffset = 0.01f;

typedef struct TagBufferMaterial {
	float3 _diffuseColor;
	float _specularColor;
	float _shininess;
	float _emissiveColor;

	// An index of 0 means unused
	ushort _diffuseMapIndexPlusOne;
	ushort _specularMapIndexPlusOne;
	ushort _shininessMapIndexPlusOne;
	ushort _emissiveMapIndexPlusOne;

	// Special
	ushort _normalMapIndexPlusOne;
} BufferMaterial;

typedef struct TagVertex {
	float3 _position;
	float3 _normal;
	float2 _texCoord;
} Vertex;

typedef struct TagBufferTriangle {
	Vertex _v0;
	Vertex _v1;
	Vertex _v2;

	ushort _materialIndex;
} BufferTriangle;

typedef struct TagBufferTreeNode {
	float3 _minBounds;
	float3 _maxBounds;

	ushort _parentIndexPlusOne;

	ushort _childIndexPlusOne0;
	ushort _childIndexPlusOne1;

	ushort _trianglesStartIndex;
	ushort _numTriangles;
} BufferTreeNode;

typedef struct TagBufferLight {
	ushort _type;
	float3 _position;
	float3 _color;
	float3 _direction;
	float _range;
	float _rangeInv;
	float _lightSpreadAngleCos;
	float _lightSpreadAngleCosFlipInv;
	float _lightExponent;
} BufferLight;

inline float rand(float2 co) {
	float f;
    return 2.0f * fract(sin(dot(co.xy + (float2)(randSeed), (float2)(12.9898f, 78.2331f))) * 43758.5453f, &f) - 1.0f;
}

inline float3 rand3(float2 co) {
	co *= 223.2132f;
	return (float3)(rand(co), rand(co + (float2)(165.2328f, 972.7544f)), rand(co - (float2)(113.2328f, 482.3551f)));
}

inline float3 reflect(float3 V, float3 N) {
	return V - 2.0f * dot( V, N ) * N;
}

inline float3 refract(float3 V, float3 N, float refrIndex) {
	float cosI = -dot( N, V );
	float cosT2 = 1.0f - refrIndex * refrIndex * (1.0f - cosI * cosI);
	return (refrIndex * V) + (refrIndex * cosI - sqrt( cosT2 )) * N;
}

bool intersectsTriangle(float3 p1, float3 p2, float3 p3, float3 start, float3 dir, float* t) {
	float3 e1 = p2 - p1;
	float3 e2 = p3 - p1;

	float3 h = cross(dir, e2);
	float a = dot(e1, h);

	if (fabs(a) < 0.001f)
		return false;

	float f = 1.0f / a;

	float3 s = start - p1;

	float u = f * dot(s, h);

	if (u < 0.0f || u > 1.0f)
		return false;

	float3 q = cross(s, e1);

	float v = f * dot(dir, q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	float tV = f * dot(e2, q);

	if (tV > 0.001f) {
		(*t) = tV;
		
		return true;
	}

	return false;
}

float3 getBarycentricCoords(float3 p1, float3 p2, float3 p3, float3 hitPos) {
	// Find barycentric coordinates (area-weighted coordinates of (*hitPos))
	float3 f0 = p1 - hitPos;
	float3 f1 = p2 - hitPos;
	float3 f2 = p3 - hitPos;

	float3 vecArea = cross(p1 - p2, p1 - p3);
	float3 vecArea0 = cross(f1, f2);
	float3 vecArea1 = cross(f2, f0);
	float3 vecArea2 = cross(f0, f1);

	float areaInv = 1.0f / length(vecArea);
	float area0 = length(vecArea0) * areaInv * (dot(vecArea, vecArea0) > 0.0f ? 1.0f : -1.0f);
	float area1 = length(vecArea1) * areaInv * (dot(vecArea, vecArea1) > 0.0f ? 1.0f : -1.0f);
	float area2 = length(vecArea2) * areaInv * (dot(vecArea, vecArea2) > 0.0f ? 1.0f : -1.0f);

	return (float3)(area0, area1, area2);
}

inline bool aabbContainsPoint(float3 lower, float3 upper, float3 p) {
	return p.x > lower.x && p.y > lower.y && p.z > lower.z && p.x < upper.x && p.y < upper.y && p.z < upper.z;
}

bool aabbIntersectionPoints(float3 start, float3 direction, float3 minBounds, float3 maxBounds, float* t0, float* t1) {
	float3 directionInv = 1.0f / direction;

	float tx1 = (minBounds.x - start.x) * directionInv.x;
	float tx2 = (maxBounds.x - start.x) * directionInv.x;

	(*t0) = min(tx1, tx2);
	(*t1) = max(tx1, tx2);

	float ty1 = (minBounds.y - start.y) * directionInv.y;
	float ty2 = (maxBounds.y - start.y) * directionInv.y;

	(*t0) = max((*t0), min(ty1, ty2));
	(*t1) = min((*t1), max(ty1, ty2));

	float tz1 = (minBounds.z - start.z) * directionInv.z;
	float tz2 = (maxBounds.z - start.z) * directionInv.z;

	(*t0) = max((*t0), min(tz1, tz2));
	(*t1) = min((*t1), max(tz1, tz2));

	return (*t1) >= (*t0);
}

bool rayCast(global const BufferTreeNode* treeBuffer, global const BufferTriangle* triangleBuffer, global const BufferMaterial* materialBuffer, read_only image2d_array_t materialTextures,
	float3 start, float3 dir, float3* hitPos, float3* hitNormal, ushort* materialIndex)
{
	float minT = 99999.0f;
	int minTriangleIndex;

	// Find initial query point
	float3 currentLower = treeBuffer[0]._minBounds;
	float3 currentUpper = treeBuffer[0]._maxBounds;

	float3 queryPoint = start;

	int currentNodeIndex = 0;

	bool hit = false;
	
	// Safety counter
	for (int s = 0; s < maxKDSteps; s++) {
		ushort childIndex1 = treeBuffer[currentNodeIndex]._childIndexPlusOne0;
		ushort childIndex2 = treeBuffer[currentNodeIndex]._childIndexPlusOne1;
		
		// Go through child containing the query point if it has children. If not (and no triangles were previously hit), find a new query point
		if (childIndex1 != 0) {
			childIndex1--;
			childIndex2--;
			
			float3 lower1 = treeBuffer[childIndex1]._minBounds;
			float3 upper1 = treeBuffer[childIndex1]._maxBounds;

			if (aabbContainsPoint(lower1, upper1, queryPoint)) {
				// Continue search from this node
				currentNodeIndex = childIndex1;
				currentLower = lower1;
				currentUpper = upper1;
			}
			else {
				float3 lower2 = treeBuffer[childIndex2]._minBounds;
				float3 upper2 = treeBuffer[childIndex2]._maxBounds;

				if (aabbContainsPoint(lower2, upper2, queryPoint)) {
					// Continue search from this node
					currentNodeIndex = childIndex2;
					currentLower = lower2;
					currentUpper = upper2;
				}
				else
					break;
			}
		}
		else {
			ushort trianglesStartIndex = treeBuffer[currentNodeIndex]._trianglesStartIndex;
			ushort numTriangles = treeBuffer[currentNodeIndex]._numTriangles;
			
			// Query all triangles in current node
			for (int i = 0; i < numTriangles && i < maxTriangles; i++) {
				int j = trianglesStartIndex + i;

				float3 pt1 = triangleBuffer[j]._v0._position;
				float3 pt2 = triangleBuffer[j]._v1._position;
				float3 pt3 = triangleBuffer[j]._v2._position;

				float t;

				if (intersectsTriangle(pt1, pt2, pt3, start, dir, &t)) {
					if (t < minT) {
						minT = t;
						minTriangleIndex = j;
					}

					hit = true;
				}
			}

			if (hit)
				break;

			// Still no hits, continue search
			// Ray cast aabb to find new point
			float ct0, ct1;

			aabbIntersectionPoints(start, dir, currentLower, currentUpper, &ct0, &ct1);

			queryPoint = start + dir * (ct1 + queryOffset);
			//queryPoint = (currentLower + currentUpper) * 0.5 + dir * length(currentUpper - currentLower) * 0.5;

			// Go up to continue search
			currentNodeIndex = 0;

			currentLower = treeBuffer[currentNodeIndex]._minBounds;
			currentUpper = treeBuffer[currentNodeIndex]._maxBounds;

			if (!aabbContainsPoint(currentLower, currentUpper, queryPoint))
				break;
		}
	}

	if (hit) {
		(*hitPos) = start + dir * minT;

		float3 p1 = triangleBuffer[minTriangleIndex]._v0._position;
		float3 p2 = triangleBuffer[minTriangleIndex]._v1._position;
		float3 p3 = triangleBuffer[minTriangleIndex]._v2._position;

		float3 bary = getBarycentricCoords(p1, p2, p3, (*hitPos));

		(*hitNormal) = 
			triangleBuffer[minTriangleIndex]._v0._normal * bary.x + 
			triangleBuffer[minTriangleIndex]._v1._normal * bary.y + 
			triangleBuffer[minTriangleIndex]._v2._normal * bary.z;

		//hitNormal = texelFetch(triBuffer, minTriangleIndex + 1).xyz;

		(*materialIndex) = triangleBuffer[minTriangleIndex]._materialIndex;
	}

	return hit;
}

bool rayCastShadow(global const BufferTreeNode* treeBuffer, global const BufferTriangle* triangleBuffer, global const BufferMaterial* materialBuffer, read_only image2d_array_t materialTextures,
	float3 start, float3 dir, float refDistance)
{
	// Find initial query point
	float3 currentLower = treeBuffer[0]._minBounds;
	float3 currentUpper = treeBuffer[0]._maxBounds;

	float rootT0, rootT1;

	if (!aabbIntersectionPoints(start, dir, currentLower, currentUpper, &rootT0, &rootT1))
		return false;

	float3 queryPoint = start + dir * (rootT0 + testOffset);

	int currentNodeIndex = 0;

	// Safety counter
	for (int s = 0; s < maxKDSteps; s++) {
		ushort childIndex1 = treeBuffer[currentNodeIndex]._childIndexPlusOne0;
		ushort childIndex2 = treeBuffer[currentNodeIndex]._childIndexPlusOne1;

		// Go through child containing the query point if it has children. If not (and no triangles were previously hit), find a new query point
		if (childIndex1 != 0) {
			childIndex1--;
			childIndex2--;
			
			float3 lower1 = treeBuffer[childIndex1]._minBounds;
			float3 upper1 = treeBuffer[childIndex1]._maxBounds;

			if (aabbContainsPoint(lower1, upper1, queryPoint)) {
				// Continue search from this node
				currentNodeIndex = childIndex1;
				currentLower = lower1;
				currentUpper = upper1;
			}
			else {
				float3 lower2 = treeBuffer[childIndex2]._minBounds;
				float3 upper2 = treeBuffer[childIndex2]._maxBounds;

				if (aabbContainsPoint(lower2, upper2, queryPoint)) {
					// Continue search from this node
					currentNodeIndex = childIndex2;
					currentLower = lower2;
					currentUpper = upper2;
				}
				else
					return false;
			}
		}
		else {
			ushort trianglesStartIndex = treeBuffer[currentNodeIndex]._trianglesStartIndex;
			ushort numTriangles = treeBuffer[currentNodeIndex]._numTriangles;
			
			// Query all triangles in current node
			for (int i = 0; i < numTriangles && i < maxTriangles; i++) {
				int j = trianglesStartIndex + i;

				float3 pt1 = triangleBuffer[j]._v0._position;
				float3 pt2 = triangleBuffer[j]._v1._position;
				float3 pt3 = triangleBuffer[j]._v2._position;

				float t;

				if (intersectsTriangle(pt1, pt2, pt3, start, dir, &t))
					if (t < refDistance)
						return true;
			}

			// Still no hits, continue search
			// Ray cast aabb to find new point
			float ct0, ct1;

			aabbIntersectionPoints(start, dir, currentLower, currentUpper, &ct0, &ct1);

			queryPoint = start + dir * (ct1 + queryOffset);

			currentNodeIndex = 0;
		}
	}

	return false;
}

float3 quaternionTransform(float4 quat, float3 vec) {
	float3 quatVec = (float3)(quat.x, quat.y, quat.z);
	float3 uv = cross(quatVec, vec);
	float3 uuv = cross(quatVec, uv);
	uv *= 2.0 * quat.w;
	uuv *= 2.0;

	return vec + uv + uuv;
}

float3 matrix4x4PointTransform(global const float* matrix, float3 point) {
	float3 result;

	result.x = matrix[0] * point.x + matrix[4] * point.y + matrix[8] * point.z + matrix[12];
	result.y = matrix[1] * point.x + matrix[5] * point.y + matrix[9] * point.z + matrix[13];
	result.z = matrix[2] * point.x + matrix[6] * point.y + matrix[10] * point.z + matrix[14];
	
	return result;
}

float3 matrix3x3PointTransform(global const float* matrix, float3 point) {
	float3 result;
	
	result.x = matrix[0] * point.x + matrix[3] * point.y + matrix[6] * point.z;
	result.y = matrix[1] * point.x + matrix[4] * point.y + matrix[7] * point.z;
	result.z = matrix[2] * point.x + matrix[5] * point.y + matrix[8] * point.z;

	return result;
}

void kernel globalIllumination(
	global const BufferTreeNode* treeBuffer,
	global const BufferTriangle* triangleBuffer,
	global const BufferMaterial* materialBuffer,
	global const BufferLight* lightBuffer,
	constant const float* viewInverseMatrix,
	constant const float* normalInverseMatrix,
	read_only image2d_array_t materialTextures,
	const uint numLights,
	read_only image2d_t gBufferPosition, read_only image2d_t gBufferNormal, read_only image2d_t gBufferColor,
	write_only image2d_t target, uint2 targetSize)
{
	int2 pixelPos = (int2)(get_global_id(0), get_global_id(1));
	
	float2 gTexCoord = (float2)((float)pixelPos.x / (float)targetSize.x, (float)pixelPos.y / (float)targetSize.y);

	float3 hitPos = matrix4x4PointTransform(viewInverseMatrix, read_imagef(gBufferPosition, normalizedClampedNearestSampler, gTexCoord).xyz);

	float3 hitNormal = normalize(matrix3x3PointTransform(normalInverseMatrix, read_imagef(gBufferPosition, normalizedClampedNearestSampler, gTexCoord).xyz));

	float3 reflected = hitNormal;

	float4 diffAndSpec = read_imagef(gBufferColor, normalizedClampedNearestSampler, gTexCoord);
	float3 diffuseColor = (float3)(1.0f, 1.0f, 1.0f);
	float3 materialData = (float3)(diffAndSpec.w, 0.0f, 1.0f);

	float3 color = (float3)(0.0f, 0.0f, 0.0f);

	// Samples
	for (int i = 0; i < numPathSamples; i++) {
		float3 prevColor = diffuseColor;
		float3 subColor = (float3)(0.0f, 0.0f, 0.0f);

		float3 bounceHitPos;
		float3 bounceHitNormal;
		ushort bounceMaterialIndex;

		float3 bounceDiffuseColor = diffuseColor;
		float3 bounceMaterialData = materialData;

		float bounceInfluence = bounceMaterialData.x;

		for (int j = 0; j < numAdditionalBounces; j++) {
			float3 dir = normalize(reflected + normalize(rand3(gTexCoord + (float2)((float)j * 43.0f + (float)i * 195.0f))) * (1.0f - bounceMaterialData.y));

			if (rayCast(treeBuffer, triangleBuffer, materialBuffer, materialTextures, hitPos + hitNormal * testOffset, dir, &bounceHitPos, &bounceHitNormal, &bounceMaterialIndex)) {
				write_imagef(target, pixelPos, (float4)(0.0f, 0.0f, 1.0f, 1.0f));
				
				return;
				
				float3 lightColor = (float3)(0.0f, 0.0f, 0.0f);

				for (int l = 0; l < numLights && l < maxLights; l++) {
					float3 offsetLightPosition = lightBuffer[l]._position + lightJitterDist * normalize(rand3(gTexCoord + (float2)(i * 23.0f + j * 76.0f + l * 195.0f)));
					float3 toLight = offsetLightPosition - bounceHitPos;
					float dist = length(toLight);
					toLight /= dist + 0.0001f;

#ifdef BOUNCE_SHADOW_TEST
					if(!rayCastShadow(treeBuffer, triangleBuffer, materialBuffer, materialTextures, offsetLightPosition, -toLight, dist - testOffset))
#endif
					{
						float intensity = max(0.0f, dot(toLight, bounceHitNormal)) * (lightBuffer[l]._range - dist) * lightBuffer[l]._rangeInv * (1.0f / (attenuation.x + dist * attenuation.y + dist * dist * attenuation.z));
						
						lightColor += lightBuffer[l]._color * intensity; 
					}
				}

				prevColor = bounceDiffuseColor;

				bounceDiffuseColor = materialBuffer[bounceMaterialIndex]._diffuseColor;
				bounceMaterialData = (float3)(materialBuffer[bounceMaterialIndex]._specularColor, materialBuffer[bounceMaterialIndex]._shininess, 1.0f);

				subColor += lightColor * bounceDiffuseColor * prevColor * bounceInfluence * max(0.0f, dot(bounceHitNormal, -dir));

				bounceInfluence *= bounceMaterialData.x;

				hitPos = bounceHitPos;
				hitNormal = bounceHitNormal;

				reflected = reflect(dir, bounceHitNormal);
			}
			else
				break;
		}

		color += subColor;
	}

	color /= (float)(numPathSamples);

	write_imagef(target, pixelPos, (float4)(color.x, color.y, color.z, 1.0f));
}