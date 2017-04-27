/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashBitmapTexture_H
#define traktor_flash_FlashBitmapTexture_H

#include "Flash/FlashBitmap.h"
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
class T_DLLCLASS FlashBitmapTexture : public FlashBitmap
{
	T_RTTI_CLASS;

public:
	FlashBitmapTexture();

	FlashBitmapTexture(render::ISimpleTexture* texture);

	render::ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< render::ISimpleTexture > m_texture;
};

	}
}

#endif	// traktor_flash_FlashBitmapTexture_H
