#pragma once

#include "Core/Ref.h"
#include "Flash/Bitmap.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;

	}

	namespace flash
	{

/*! \brief Flash bitmap container.
 * \ingroup Flash
 */
class T_DLLCLASS BitmapTexture : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapTexture();

	BitmapTexture(render::ISimpleTexture* texture);

	render::ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< render::ISimpleTexture > m_texture;
};

	}
}

