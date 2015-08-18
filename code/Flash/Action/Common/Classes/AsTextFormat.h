#ifndef traktor_flash_AsTextFormat_H
#define traktor_flash_AsTextFormat_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class FlashTextFormat;

/*! \brief TextFormat class.
 * \ingroup Flash
 */
class AsTextFormat : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextFormat(ActionContext* context);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual void initialize(ActionObject* self);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	std::string TextFormat_get_align(FlashTextFormat* self) const;

	void TextFormat_set_align(FlashTextFormat* self, const std::string& value) const;

	avm_number_t TextFormat_get_blockIndent(FlashTextFormat* self) const;

	void TextFormat_set_blockIndent(FlashTextFormat* self, avm_number_t value) const;

	bool TextFormat_get_bold(FlashTextFormat* self) const;

	void TextFormat_set_bold(FlashTextFormat* self, bool value) const;

	bool TextFormat_get_bullet(FlashTextFormat* self) const;

	void TextFormat_set_bullet(FlashTextFormat* self, bool value) const;

	ActionValue TextFormat_get_color(FlashTextFormat* self) const;

	void TextFormat_set_color(FlashTextFormat* self, const ActionValue& value) const;

	std::string TextFormat_get_font(FlashTextFormat* self) const;

	void TextFormat_set_font(FlashTextFormat* self, const std::string& value) const;

	avm_number_t TextFormat_get_indent(FlashTextFormat* self) const;

	void TextFormat_set_indent(FlashTextFormat* self, avm_number_t value) const;

	bool TextFormat_get_italic(FlashTextFormat* self) const;

	void TextFormat_set_italic(FlashTextFormat* self, bool value) const;

	bool TextFormat_get_kerning(FlashTextFormat* self) const;

	void TextFormat_set_kerning(FlashTextFormat* self, bool value) const;

	avm_number_t TextFormat_get_leading(FlashTextFormat* self) const;

	void TextFormat_set_leading(FlashTextFormat* self, avm_number_t value) const;

	avm_number_t TextFormat_get_leftMargin(FlashTextFormat* self) const;

	void TextFormat_set_leftMargin(FlashTextFormat* self, avm_number_t value) const;

	avm_number_t TextFormat_get_letterSpacing(FlashTextFormat* self) const;

	void TextFormat_set_letterSpacing(FlashTextFormat* self, avm_number_t value) const;

	avm_number_t TextFormat_get_rightMargin(FlashTextFormat* self) const;

	void TextFormat_set_rightMargin(FlashTextFormat* self, avm_number_t value) const;

	avm_number_t TextFormat_get_size(FlashTextFormat* self) const;

	void TextFormat_set_size(FlashTextFormat* self, avm_number_t value) const;

	ActionValue TextFormat_get_tabStops(FlashTextFormat* self) const;

	void TextFormat_set_tabStops(FlashTextFormat* self, const ActionValue& value) const;

	std::string TextFormat_get_target(FlashTextFormat* self) const;

	void TextFormat_set_target(FlashTextFormat* self, const std::string& value) const;

	bool TextFormat_get_underline(FlashTextFormat* self) const;

	void TextFormat_set_underline(FlashTextFormat* self, bool value) const;

	std::string TextFormat_get_url(FlashTextFormat* self) const;

	void TextFormat_set_url(FlashTextFormat* self, const std::string& value) const;
};

	}
}

#endif	// traktor_flash_AsTextFormat_H
