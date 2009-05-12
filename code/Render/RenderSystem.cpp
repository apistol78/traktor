#include <limits>
#include <cmath>
#include "Render/RenderSystem.h"
#include "Render/DisplayMode.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderSystem", RenderSystem, Object)

DisplayMode* RenderSystem::findDisplayMode(const DisplayMode* criteria)
{
	int bestMatch = std::numeric_limits< int >::max();
	Ref< DisplayMode > best;
	
	// Try to find the best matching display mode using a weight criteria.
	for (int i = 0; i < getDisplayModeCount(); ++i)
	{
		Ref< DisplayMode > check = getDisplayMode(i);
		if (check != 0)
		{
			int match =
				std::abs(
					(int)(check->getWidth() - criteria->getWidth()) +
					(int)(check->getHeight() - criteria->getHeight()) +
					(int)(check->getColorBits() - criteria->getColorBits()) * 10
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
