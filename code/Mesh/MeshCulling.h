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

/*! Enable screen-space culling; quite expensive but might yield less draw calls. */
#if !defined(__PS3__)
#	define T_ENABLE_SCREENSPACE_CULLING 1
#endif

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
	const Aabb3& meshBoundingBox,
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

	if (frustum.inside(center, radius) == Frustum::IrOutside)
		return false;

#if T_ENABLE_SCREENSPACE_CULLING

	if (minScreenArea > FUZZY_EPSILON)
	{
		// Project bounding box extents onto view plane.
		Vector4 extents[8];
		meshBoundingBox.getExtents(extents);

		Vector4 mn(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max()
		);
		Vector4 mx(
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max()
		);

		Matrix44 worldViewProj = projection * worldView;

		for (int i = 0; i < sizeof_array(extents); ++i)
		{
			Vector4 p = worldViewProj * extents[i];
			if (p.w() <= 0.0f)
			{
				// Bounding box clipped to view plane; assume it's visible.
				outDistance = center.z() + radius;
				return true;
			}

			// Homogeneous divide.
			p /= p.w();

			// Track screen space extents.
			mn = min(mn, p);
			mx = max(mx, p);
		}

		// Ensure we're visible.
		if (mn.x() > 1.0f || mn.y() > 1.0f || mx.x() < -1.0f || mx.y() < -1.0f)
			return false;

		// Calculate screen area, cull if it's below threshold.
		Vector4 e = mx - mn;
		if (e.x() * e.y() < minScreenArea)
			return false;
	}

#endif

	outDistance = center.z() + radius;
	return true;
}

	}
}

#endif	// traktor_mesh_MeshCulling_H
