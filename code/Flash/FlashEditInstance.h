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
class TextLayout;

/*! \brief Dynamic text character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashEditInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashEditInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html);

	const FlashEdit* getEdit() const;

	bool parseText(const std::wstring& text);

	bool parseHtml(const std::wstring& html);

	const SwfColor& getTextColor() const;

	void setTextColor(const SwfColor& textColor);

	float getLetterSpacing() const;

	void setLetterSpacing(float letterSpacing);

	Ref< FlashTextFormat > getTextFormat() const;

	Ref< FlashTextFormat > getTextFormat(int32_t beginIndex, int32_t endIndex) const;

	void setTextFormat(const FlashTextFormat* textFormat);

	void setTextFormat(const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex);

	std::wstring getText() const;

	std::wstring getHtmlText() const;

	int32_t getCaret() const;

	void setScroll(int32_t scroll);

	int32_t getScroll() const;

	int32_t getMaxScroll() const;

	const TextLayout* getTextLayout() const;

	virtual Aabb2 getBounds() const;

	virtual void eventKey(wchar_t unicode);

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button);

private:
	mutable Semaphore m_lock;
	Ref< const FlashEdit > m_edit;
	SwfColor m_textColor;
	float m_letterSpacing;
	float m_fontHeight;
	std::wstring m_text;
	std::wstring m_htmlText;
	bool m_html;
	int32_t m_caret;
	int32_t m_scroll;
	Ref< TextLayout > m_layout;

	bool internalParseText(const std::wstring& text);

	bool internalParseHtml(const std::wstring& html);

	void updateLayout();
};

	}
}

#endif	// traktor_flash_FlashEditInstance_H
