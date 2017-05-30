/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_BitmapResource_H
#define traktor_flash_BitmapResource_H

#include "Core/Guid.h"
#include "Flash/Bitmap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Flash bitmap container.
 * \ingroup Flash
 */
class T_DLLCLASS BitmapResource : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapResource();

	BitmapResource(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t atlasWidth, uint32_t atlasHeight, const Guid& resourceId);

	uint32_t getAtlasWidth() const { return m_atlasWidth; }

	uint32_t getAtlasHeight() const { return m_atlasHeight; }

	const Guid& getResourceId() const { return m_resourceId; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_atlasWidth;
	uint32_t m_atlasHeight;
	Guid m_resourceId;
};

	}
}

#endif	// traktor_flash_BitmapResource_H
