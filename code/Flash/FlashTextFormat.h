#ifndef traktor_flash_FlashTextFormat_H
#define traktor_flash_FlashTextFormat_H

#include "Flash/SwfTypes.h"
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
	FlashTextFormat(float letterSpacing, SwfTextAlignType align, float size);

	void setLetterSpacing(float letterSpacing) { m_letterSpacing = letterSpacing; }

	float getLetterSpacing() const { return m_letterSpacing; }

	void setAlign(SwfTextAlignType align) { m_align = align; }

	SwfTextAlignType getAlign() const { return m_align; }

	void setSize(float size) { m_size = size; }

	float getSize() const { return m_size; }

private:
	float m_letterSpacing;
	SwfTextAlignType m_align;
	float m_size;
};

	}
}

#endif	// traktor_flash_FlashTextFormat_H
