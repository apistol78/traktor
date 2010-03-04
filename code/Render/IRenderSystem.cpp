#include <limits>
#include "Core/Math/MathUtils.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IRenderSystem", IRenderSystem, Object)

bool IRenderSystem::findDisplayMode(const DisplayMode& criteria, DisplayMode& outBestMatch) const
{
	int32_t bestMatch = std::numeric_limits< int32_t >::max();
	
	for (uint32_t i = 0; i < getDisplayModeCount(); ++i)
	{
		DisplayMode check = getDisplayMode(i);
		int32_t match =
			abs< int32_t >(
				(int32_t)(check.width - criteria.width) +
				(int32_t)(check.height - criteria.height) +
				(int32_t)(check.colorBits - criteria.colorBits) * 10
			);
		if (match < bestMatch)
		{
			outBestMatch = check;
			bestMatch = match;
		}
	}
	
	return bestMatch != std::numeric_limits< int32_t >::max();
}

	}
}
