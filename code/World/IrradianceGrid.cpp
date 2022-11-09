#include <cstring>
#include "World/IrradianceGrid.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGrid", IrradianceGrid, Object)

IrradianceGrid::IrradianceGrid(
	gridSize_t size,
	const Aabb3& boundingBox,
	render::Buffer* buffer
)
:   m_boundingBox(boundingBox)
,   m_buffer(buffer)
{
	std::memcpy(m_size, size, sizeof(gridSize_t));
}

}
