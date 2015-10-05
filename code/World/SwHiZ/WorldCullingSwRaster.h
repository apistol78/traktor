#ifndef traktor_world_WorldCullingSwRaster_H
#define traktor_world_WorldCullingSwRaster_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/AutoPtr.h"
#include "World/IWorldCulling.h"

namespace traktor
{
	namespace world
	{

class WorldRenderView;

/*! \brief
 * \ingroup World
 */
class WorldCullingSwRaster : public IWorldCulling
{
	T_RTTI_CLASS;

public:
	WorldCullingSwRaster();

	void beginPrecull(const WorldRenderView& worldRenderView);

	void endPrecull();

	virtual void placeOccluder(const OccluderMesh* mesh, const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool queryAabb(const Aabb3& aabb, const Transform& transform) const T_OVERRIDE T_FINAL;

private:
	AutoArrayPtr< uint16_t > m_depth[16];
	AlignedVector< Vector4 > m_clipVertices;
	Matrix44 m_projection;
	Matrix44 m_view;
};

	}
}

#endif	// traktor_world_WorldCullingSwRaster_H
