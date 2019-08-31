#include "World/IrradianceGrid.h"

namespace traktor
{
    namespace world
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGrid", IrradianceGrid, Object)

IrradianceGrid::IrradianceGrid(const Vector4& size, render::StructBuffer* buffer)
:   m_size(size)
,   m_buffer(buffer)
{
}

    }
}