/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashBitmap_H
#define traktor_flash_FlashBitmap_H

#include "Core/Serialization/ISerializable.h"

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
class T_DLLCLASS FlashBitmap : public ISerializable
{
	T_RTTI_CLASS;

public:
	FlashBitmap();

	FlashBitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	/*! \brief Set cache object.
	*/
	void setCacheObject(IRefCount* cacheObject) const;

	/*! \brief Set cache object.
	*/
	IRefCount* getCacheObject() const { return m_cacheObject; }

	uint32_t getX() const { return m_x; }

	uint32_t getY() const { return m_y; }

	uint32_t getWidth() const { return m_width; }

	uint32_t getHeight() const { return m_height; }

	virtual void serialize(ISerializer& s) T_OVERRIDE;

protected:
	mutable Ref< IRefCount > m_cacheObject;
	uint32_t m_x;
	uint32_t m_y;
	uint32_t m_width;
	uint32_t m_height;
};

	}
}

#endif	// traktor_flash_FlashBitmap_H
