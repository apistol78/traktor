#ifndef traktor_flash_FlashEditInstance_H
#define traktor_flash_FlashEditInstance_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Flash/FlashCharacterInstance.h"

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

class FlashEdit;
class FlashTextFormat;

/*! \brief Dynamic text character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashEditInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	typedef std::list< std::wstring > text_t;

	FlashEditInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html);

	const FlashEdit* getEdit() const;

	bool parseText(const std::wstring& text);

	bool parseHtml(const std::wstring& html);

	Ref< FlashTextFormat > getTextFormat() const;

	Ref< FlashTextFormat > getTextFormat(int32_t beginIndex, int32_t endIndex) const;

	void setTextFormat(const FlashTextFormat* textFormat);

	void setTextFormat(const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex);

	text_t getText() const;

	std::wstring getConcatedText() const;

	bool getTextExtents(float& outWidth, float& outHeight) const;

	virtual SwfRect getBounds() const;

	virtual void eventKey(wchar_t unicode);

	const SwfColor& getTextColor() const { return m_textColor; }

	void setTextColor(const SwfColor& textColor) { m_textColor = textColor; }

	void setLetterSpacing(float letterSpacing) { m_letterSpacing = letterSpacing; }

	float getLetterSpacing() const { return m_letterSpacing; }

private:
	mutable Semaphore m_lock;
	Ref< const FlashEdit > m_edit;
	SwfColor m_textColor;
	float m_letterSpacing;
	text_t m_text;
};

	}
}

#endif	// traktor_flash_FlashEditInstance_H
