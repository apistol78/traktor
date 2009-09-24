#include <limits>
#include "Render/IRenderSystem.h"
#include "Render/DisplayMode.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IRenderSystem", IRenderSystem, Object)

DisplayMode* IRenderSystem::findDisplayMode(const DisplayMode* criteria)
{
	int bestMatch = std::numeric_limits< int >::max();
	Ref< DisplayMode > best;
	
	// Try to find the best matching display mode using a weight criteria.
	for (int i = 0; i < getDisplayModeCount(); ++i)
	{
		Ref< DisplayMode > check = getDisplayMode(i);
		if (check != 0)
		{
			int32_t match =
				abs< int32_t >(
					(int32_t)(check->getWidth() - criteria->getWidth()) +
					(int32_t)(check->getHeight() - criteria->getHeight()) +
					(int32_t)(check->getColorBits() - criteria->getColorBits()) * 10
				);
			if (match == 0)
			{
				// Better than this isn't possible.
				best = check;
				break;
			}
			else
			{
				// Is it better than any previously found.
				if (best == 0 || match < bestMatch)
				{
					best = check;
					bestMatch = match;
				}
			}
		}
	}
	
	return best;
}

	}
}
