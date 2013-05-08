#ifndef traktor_flash_FlashEdit_H
#define traktor_flash_FlashEdit_H

#include "Flash/FlashCharacter.h"
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
	namespace flash
	{

/*! \brief Dynamic text field.
 * \ingroup Flash
 */
class T_DLLCLASS FlashEdit : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	enum Align
	{
		AnLeft = 0x00,
		AnRight = 0x01,
		AnCenter = 0x02,
		AnJustify = 0x03
	};

	FlashEdit();

	FlashEdit(
		uint16_t id,
		uint16_t fontId,
		uint16_t fontHeight,
		const SwfRect& textBounds,
		const SwfColor& textColor,	
		const std::wstring& initialText,
		Align align,
		uint16_t leftMargin,
		uint16_t rightMargin,
		bool wordWrap,
		bool multiLine,
		bool renderHtml
	);

	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashCharacterInstance* parent,
		const std::string& name,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const;

	/*! \brief Get font identifier.
	 *
	 * \return Font identifier.
	 */
	uint16_t getFontId() const;

	/*! \brief Get font height.
	 *
	 * \return Font height.
	 */
	uint16_t getFontHeight() const;

	/*! \brief Get text character bounding box.
	 *
	 * \return Bounding box.
	 */
	const SwfRect& getTextBounds() const;

	/*! \brief Get text color.
	 *
	 * \return Text color.
	 */
	const SwfColor& getTextColor() const;

	/*! \brief Initial text string.
	 *
	 * \return Initial text, can be HTML.
	 */
	const std::wstring& getInitialText() const;

	/*! \brief Get text alignment within bounding box.
	 *
	 * \return Alignment.
	 */
	Align getAlign() const;

	/*! \brief Get left margin.
	 *
	 * \return Left margin.
	 */
	uint16_t getLeftMargin() const;

	/*! \brief Get right margin.
	 *
	 * \return Right margin.
	 */
	uint16_t getRightMargin() const;

	/*! \brief Word wrap enabled.
	 *
	 * \return True if word wrap is enabled.
	 */
	bool wordWrap() const;

	/*! \brief Multiline text field.
	 *
	 * \return True if multiline.
	 */
	bool multiLine() const;

	/*! \brief Render HTML content.
	 *
	 * \return True if text is HTML.
	 */
	bool renderHtml() const;

	virtual void serialize(ISerializer& s);

private:
	uint16_t m_fontId;
	uint16_t m_fontHeight;
	SwfRect m_textBounds;
	SwfColor m_textColor;
	std::wstring m_initialText;
	Align m_align;
	uint16_t m_leftMargin;
	uint16_t m_rightMargin;
	bool m_wordWrap;
	bool m_multiLine;
	bool m_renderHtml;
};

	}
}

#endif	// traktor_flash_FlashEdit_H
