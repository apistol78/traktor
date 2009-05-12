#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Xbox360/RenderTargetSetXbox360.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetXbox360", RenderTargetSetXbox360, RenderTargetSet)

void RenderTargetSetXbox360::destroy()
{
}

int RenderTargetSetXbox360::getWidth() const
{
	return 0;
}

int RenderTargetSetXbox360::getHeight() const
{
	return 0;
}

Texture* RenderTargetSetXbox360::getColorTexture(int index) const
{
	return 0;
}

	}
}
