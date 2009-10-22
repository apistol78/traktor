#ifndef traktor_mesh_MeshCulling_H
#define traktor_mesh_MeshCulling_H

#include <limits>
#include "Core/Math/Vector2.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace mesh
	{

/*! Enable screenspace culling; quite expensive but might yield less draw calls. */
#define T_ENABLE_SCREENSPACE_CULLING 0

/*! \brief Mesh culling.
 * First it performs a grosse culling
 * with the view frustum, then it
 * will estimate screen space area
 * from the bounding box in order to determine
 * if mesh is worth rendering.
 *
 * \param meshBoundingBox Bounding box of mesh.
 * \param frustum View culling frustum.
 * \param worldView World-view transformation.
 * \param projection Projection transformation.
 * \param minScreenArea Minimum screen area in pre-viewport space.
 * \param outDistance Distance from origo if visible.
 * \return True if mesh is deemed visible.
 */
inline bool isMeshVisible(
	const Aabb& meshBoundingBox,
	const Frustum& frustum,
	const Matrix44& worldView,
	const Matrix44& projection,
	const float minScreenArea,
	float& outDistance
)
{
	// Earliest, empty bounding boxes cannot contain anything visible.
	if (meshBoundingBox.empty())
		return false;

	// Early out of bounding sphere is outside of frustum.
	Vector4 center = worldView * meshBoundingBox.getCenter().xyz1();
	Scalar radius = meshBoundingBox.getExtent().length();

	if (!frustum.inside(center, radius))
		return false;

#if T_ENABLE_SCREENSPACE_CULLING

	// Project bounding box extents onto view plane.
	Vector4 extents[8];
	meshBoundingBox.getExtents(extents);

	Vector2 mn( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max());
	Vector2 mx(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max());

	Matrix44 worldViewProj = worldView * projection;

	for (int i = 0; i < sizeof_array(extents); ++i)
	{
		Vector4 p = worldViewProj * extents[i];
		if (p.w <= 0.0f)
		{
			// Bounding box clipped to view plane; assume it's visible.
			outDistance = center.length();
			return true;
		}

		// Homogeneous divide.
		p.x /= p.w;
		p.y /= p.w;

		// Track screen space extents.
		mn.x = min(mn.x, p.x);
		mn.y = min(mn.y, p.y);
		mx.x = max(mx.x, p.x);
		mx.y = max(mx.y, p.y);
	}

	// Calculate screen area, cull if it's below threshold.
	float area = (mx.x - mn.x) * (mx.y - mn.y);
	if (area < minScreenArea)
		return false;

#endif

	outDistance = center.length();
	return true;
}

	}
}

#endif	// traktor_mesh_MeshCulling_H
