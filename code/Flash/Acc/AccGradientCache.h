#ifndef traktor_flash_AccGradientCache_H
#define traktor_flash_AccGradientCache_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace flash
	{

class AccBitmapRect;
class FlashFillStyle;

/*! \brief Gradient cache for accelerated rendering.
 * \ingroup Flash
 */
class AccGradientCache : public Object
{
public:
	AccGradientCache(render::IRenderSystem* renderSystem);

	virtual ~AccGradientCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getGradientTexture(const FlashFillStyle& style);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ISimpleTexture > m_gradientsTexture;
	SmallMap< uint64_t, Ref< AccBitmapRect > > m_cache;
	AutoArrayPtr< uint8_t > m_gradientsData;
	uint32_t m_currentGradientColumn;
	uint32_t m_nextGradient;
};

	}
}

#endif	// traktor_flash_AccGradientCache_H
