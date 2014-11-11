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

	FlashBitmap(uint32_t width, uint32_t height);

	uint32_t getWidth() const { return m_width; }

	uint32_t getHeight() const { return m_height; }

	virtual void serialize(ISerializer& s);

protected:
	uint32_t m_width;
	uint32_t m_height;
};

	}
}

#endif	// traktor_flash_FlashBitmap_H
