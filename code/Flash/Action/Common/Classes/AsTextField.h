/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsTextField_H
#define traktor_flash_AsTextField_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Array;
class FlashContextMenu;
class EditInstance;
class SpriteInstance;
class FlashStyleSheet;
class TextFormat;

/*! \brief TextField class.
 * \ingroup Flash
 */
class AsTextField : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextField(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void TextField_addListener(EditInstance* editInstance, ActionObject* listener) const;

	float TextField_getDepth(EditInstance* editInstance) const;

	Ref< Array > TextField_getFontList(EditInstance* editInstance) const;

	Ref< TextFormat > TextField_getNewTextFormat(EditInstance* editInstance) const;

	Ref< TextFormat > TextField_getTextFormat_0(EditInstance* editInstance) const;

	Ref< TextFormat > TextField_getTextFormat_2(EditInstance* editInstance, int32_t beginIndex, int32_t endIndex) const;

	bool TextField_removeListener(EditInstance* editInstance) const;

	void TextField_removeTextField(EditInstance* editInstance) const;

	void TextField_replaceSel(EditInstance* editInstance, const std::wstring& newText) const;

	void TextField_replaceText(EditInstance* editInstance, int32_t beginIndex, int32_t endIndex, const std::wstring& newText) const;

	void TextField_setNewTextFormat(EditInstance* editInstance, TextFormat* textFormat) const;

	void TextField_setTextFormat_0(EditInstance* editInstance, TextFormat* textFormat) const;

	void TextField_setTextFormat_2(EditInstance* editInstance, int32_t beginIndex, int32_t endIndex, TextFormat* textFormat) const;

	std::string TextField_toString(const EditInstance* self) const;

	float TextField_get_alpha(EditInstance* editInstance) const;

	void TextField_set_alpha(EditInstance* editInstance, float alpha) const;

	std::wstring TextField_get_antiAliasType(EditInstance* editInstance) const;

	void TextField_set_antiAliasType(EditInstance* editInstance, const std::wstring& antiAliasType) const;

	Ref< ActionObject > TextField_get_autoSize(EditInstance* editInstance) const;

	void TextField_set_autoSize(EditInstance* editInstance, ActionObject* autoSize) const;

	bool TextField_get_background(EditInstance* editInstance) const;

	void TextField_set_background(EditInstance* editInstance, bool hasBackground) const;

	float TextField_get_backgroundColor(EditInstance* editInstance) const;

	void TextField_set_backgroundColor(EditInstance* editInstance, float backgroundColor) const;

	bool TextField_get_border(EditInstance* editInstance) const;

	void TextField_set_border(EditInstance* editInstance, bool hasBorder) const;

	float TextField_get_borderColor(EditInstance* editInstance) const;

	void TextField_set_borderColor(EditInstance* editInstance, float borderColor) const;

	float TextField_get_bottomScroll(EditInstance* editInstance) const;

	bool TextField_get_condenseWhite(EditInstance* editInstance) const;

	void TextField_set_condenseWhite(EditInstance* editInstance, bool condenseWhite) const;

	bool TextField_get_embedFonts(EditInstance* editInstance) const;

	void TextField_set_embedFonts(EditInstance* editInstance, bool embedFonts) const;

	Ref< Array > TextField_get_filters(EditInstance* editInstance) const;

	void TextField_set_filters(EditInstance* editInstance, Array* filters) const;

	std::wstring TextField_get_gridFitType(EditInstance* editInstance) const;

	void TextField_set_gridFitType(EditInstance* editInstance, const std::wstring& gridFitType) const;

	float TextField_get_height(EditInstance* editInstance) const;

	void TextField_set_height(EditInstance* editInstance, float height) const;

	float TextField_get_highquality(EditInstance* editInstance) const;

	void TextField_set_highquality(EditInstance* editInstance, float highQuality) const;

	float TextField_get_hscroll(EditInstance* editInstance) const;

	void TextField_set_hscroll(EditInstance* editInstance, float hscroll) const;

	bool TextField_get_html(EditInstance* editInstance) const;

	void TextField_set_html(EditInstance* editInstance, bool html) const;

	std::wstring TextField_get_htmlText(EditInstance* editInstance) const;

	void TextField_set_htmlText(EditInstance* editInstance, const std::wstring& htmlText) const;

	int32_t TextField_get_length(EditInstance* editInstance) const;

	int32_t TextField_get_maxChars(EditInstance* editInstance) const;

	void TextField_set_maxChars(EditInstance* editInstance, int32_t maxChars) const;

	float TextField_get_maxhscroll(EditInstance* editInstance) const;

	int32_t TextField_get_maxscroll(EditInstance* editInstance) const;

	Ref< FlashContextMenu > TextField_get_menu(EditInstance* editInstance) const;

	void TextField_set_menu(EditInstance* editInstance, FlashContextMenu* contextMenu) const;

	bool TextField_get_mouseWheelEnabled(EditInstance* editInstance) const;

	void TextField_set_mouseWheelEnabled(EditInstance* editInstance, bool mouseWheelEnabled) const;

	bool TextField_get_multiline(EditInstance* editInstance) const;

	void TextField_set_multiline(EditInstance* editInstance, bool multiliine) const;

	std::string TextField_get_name(EditInstance* editInstance) const;

	void TextField_set_name(EditInstance* editInstance, const std::string& name) const;

	Ref< CharacterInstance > TextField_get_parent(EditInstance* editInstance) const;

	void TextField_set_parent(EditInstance* editInstance, SpriteInstance* movieClip) const;

	bool TextField_get_password(EditInstance* editInstance) const;

	void TextField_set_password(EditInstance* editInstance, bool password) const;

	std::wstring TextField_get_quality(EditInstance* editInstance) const;

	void TextField_set_quality(EditInstance* editInstance, const std::wstring& quality) const;

	std::wstring TextField_get_restrict(EditInstance* editInstance) const;

	void TextField_set_restrict(EditInstance* editInstance, const std::wstring& restrict) const;

	float TextField_get_rotation(EditInstance* editInstance) const;

	void TextField_set_rotation(EditInstance* editInstance, float rotation) const;

	int32_t TextField_get_scroll(EditInstance* editInstance) const;

	void TextField_set_scroll(EditInstance* editInstance, int32_t scroll) const;

	bool TextField_get_selectable(EditInstance* editInstance) const;

	void TextField_set_selectable(EditInstance* editInstance, bool selectable) const;

	float TextField_get_sharpness(EditInstance* editInstance) const;

	void TextField_set_sharpness(EditInstance* editInstance, float sharpness) const;

	float TextField_get_soundbuftime(EditInstance* editInstance) const;

	void TextField_set_soundbuftime(EditInstance* editInstance, float soundbuftime) const;

	Ref< FlashStyleSheet > TextField_get_styleSheet(EditInstance* editInstance) const;

	void TextField_set_styleSheet(EditInstance* editInstance, FlashStyleSheet* styleSheet) const;

	bool TextField_get_tabEnabled(EditInstance* editInstance) const;

	void TextField_set_tabEnabled(EditInstance* editInstance, bool tabEnabled) const;

	int32_t TextField_get_tabIndex(EditInstance* editInstance) const;

	void TextField_set_tabIndex(EditInstance* editInstance, int32_t tabIndex) const;

	std::wstring TextField_get_target(EditInstance* editInstance) const;

	std::wstring TextField_get_text(EditInstance* editInstance) const;

	void TextField_set_text(EditInstance* editInstance, const std::wstring& text) const;

	float TextField_get_textColor(EditInstance* editInstance) const;

	void TextField_set_textColor(EditInstance* editInstance, float textColor) const;

	float TextField_get_textWidth(EditInstance* editInstance) const;

	float TextField_get_textHeight(EditInstance* editInstance) const;

	float TextField_get_thickness(EditInstance* editInstance) const;

	void TextField_set_thickness(EditInstance* editInstance, float thickness) const;

	std::wstring TextField_get_type(EditInstance* editInstance) const;

	void TextField_set_type(EditInstance* editInstance, const std::wstring& type) const;

	std::wstring TextField_get_url(EditInstance* editInstance) const;

	std::string TextField_get_variable(EditInstance* editInstance) const;

	void TextField_set_variable(EditInstance* editInstance, const std::string& url) const;

	bool TextField_get_visible(EditInstance* editInstance) const;

	void TextField_set_visible(EditInstance* editInstance, bool visible) const;

	float TextField_get_width(EditInstance* editInstance) const;

	void TextField_set_width(EditInstance* editInstance, float width) const;

	bool TextField_get_wordWrap(EditInstance* editInstance) const;

	void TextField_set_wordWrap(EditInstance* editInstance, bool wordWrap) const;

	float TextField_get_x(EditInstance* editInstance) const;

	void TextField_set_x(EditInstance* editInstance, float x) const;

	float TextField_get_xmouse(EditInstance* editInstance) const;

	float TextField_get_xscale(EditInstance* editInstance) const;

	void TextField_set_xscale(EditInstance* editInstance, float xscale) const;

	float TextField_get_y(EditInstance* editInstance) const;

	void TextField_set_y(EditInstance* editInstance, float y) const;

	float TextField_get_ymouse(EditInstance* editInstance) const;

	float TextField_get_yscale(EditInstance* editInstance) const;

	void TextField_set_yscale(EditInstance* editInstance, float yscale) const;
};

	}
}

#endif	// traktor_flash_AsTextField_H
