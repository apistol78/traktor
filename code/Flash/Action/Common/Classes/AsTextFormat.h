/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsTextFormat_H
#define traktor_flash_AsTextFormat_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class TextFormat;

/*! \brief TextFormat class.
 * \ingroup Flash
 */
class AsTextFormat : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextFormat(ActionContext* context);

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	std::string TextFormat_get_align(TextFormat* self) const;

	void TextFormat_set_align(TextFormat* self, const std::string& value) const;

	float TextFormat_get_blockIndent(TextFormat* self) const;

	void TextFormat_set_blockIndent(TextFormat* self, float value) const;

	bool TextFormat_get_bold(TextFormat* self) const;

	void TextFormat_set_bold(TextFormat* self, bool value) const;

	bool TextFormat_get_bullet(TextFormat* self) const;

	void TextFormat_set_bullet(TextFormat* self, bool value) const;

	ActionValue TextFormat_get_color(TextFormat* self) const;

	void TextFormat_set_color(TextFormat* self, const ActionValue& value) const;

	std::string TextFormat_get_font(TextFormat* self) const;

	void TextFormat_set_font(TextFormat* self, const std::string& value) const;

	float TextFormat_get_indent(TextFormat* self) const;

	void TextFormat_set_indent(TextFormat* self, float value) const;

	bool TextFormat_get_italic(TextFormat* self) const;

	void TextFormat_set_italic(TextFormat* self, bool value) const;

	bool TextFormat_get_kerning(TextFormat* self) const;

	void TextFormat_set_kerning(TextFormat* self, bool value) const;

	float TextFormat_get_leading(TextFormat* self) const;

	void TextFormat_set_leading(TextFormat* self, float value) const;

	float TextFormat_get_leftMargin(TextFormat* self) const;

	void TextFormat_set_leftMargin(TextFormat* self, float value) const;

	float TextFormat_get_letterSpacing(TextFormat* self) const;

	void TextFormat_set_letterSpacing(TextFormat* self, float value) const;

	float TextFormat_get_rightMargin(TextFormat* self) const;

	void TextFormat_set_rightMargin(TextFormat* self, float value) const;

	float TextFormat_get_size(TextFormat* self) const;

	void TextFormat_set_size(TextFormat* self, float value) const;

	ActionValue TextFormat_get_tabStops(TextFormat* self) const;

	void TextFormat_set_tabStops(TextFormat* self, const ActionValue& value) const;

	std::string TextFormat_get_target(TextFormat* self) const;

	void TextFormat_set_target(TextFormat* self, const std::string& value) const;

	bool TextFormat_get_underline(TextFormat* self) const;

	void TextFormat_set_underline(TextFormat* self, bool value) const;

	std::string TextFormat_get_url(TextFormat* self) const;

	void TextFormat_set_url(TextFormat* self, const std::string& value) const;
};

	}
}

#endif	// traktor_flash_AsTextFormat_H
