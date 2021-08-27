#include "Render/Dx11/BufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferDx11", BufferDx11, Buffer)

BufferDx11::BufferDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize)
:	Buffer(elementCount, elementSize)
,	m_context(context)
{
}

	}
}