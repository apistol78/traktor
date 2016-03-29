#ifndef traktor_flash_AccBitmapRect_H
#define traktor_flash_AccBitmapRect_H

#include "Resource/Proxy.h"

namespace traktor
{
	namespace render
	{

class ISimpleTexture;

	}

	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class AccBitmapRect : public RefCountImpl< IRefCount >
{
public:
	resource::Proxy< render::ISimpleTexture > texture;
	float rect[4];

	AccBitmapRect()
	{
		rect[0] =
		rect[1] =
		rect[2] =
		rect[3] = 0.0f;
	}

	AccBitmapRect(
		const resource::Proxy< render::ISimpleTexture >& texture_,
		float left,
		float top,
		float width,
		float height
	)
	{
		texture = texture_;
		rect[0] = left;
		rect[1] = top;
		rect[2] = width;
		rect[3] = height;
	}

	bool operator == (const AccBitmapRect& rh) const
	{
		if (texture != rh.texture)
			return false;

		return
			rect[0] == rh.rect[0] &&
			rect[1] == rh.rect[1] &&
			rect[2] == rh.rect[2] &&
			rect[3] == rh.rect[3];
	}

	bool operator != (const AccBitmapRect& rh) const
	{
		return !(*this == rh);
	}
};

	}
}

#endif	// traktor_flash_AccBitmapRect_H

