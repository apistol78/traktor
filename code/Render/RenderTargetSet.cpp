#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSet", RenderTargetSet, Object)

RenderTargetSet::RenderTargetSet()
:	m_contentValid(false)
{
}

	}
}
