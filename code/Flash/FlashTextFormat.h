#ifndef traktor_flash_FlashTextFormat_H
#define traktor_flash_FlashTextFormat_H

#include "Flash/Action/ActionObjectRelay.h"

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

class T_DLLCLASS FlashTextFormat : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	FlashTextFormat(float letterSpacing);

	void setLetterSpacing(float letterSpacing) { m_letterSpacing = letterSpacing; }

	float getLetterSpacing() const { return m_letterSpacing; }

private:
	float m_letterSpacing;
};

	}
}

#endif	// traktor_flash_FlashTextFormat_H
