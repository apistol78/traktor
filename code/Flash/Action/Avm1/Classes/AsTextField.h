#ifndef traktor_flash_AsTextField_H
#define traktor_flash_AsTextField_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Array;
class FlashContextMenu;
class FlashEditInstance;
class FlashSpriteInstance;
class FlashStyleSheet;
class FlashTextFormat;

/*! \brief TextField class.
 * \ingroup Flash
 */
class AsTextField : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextField(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	void TextField_addListener(FlashEditInstance* editInstance, ActionObject* listener) const;

	avm_number_t TextField_getDepth(FlashEditInstance* editInstance) const;

	Ref< Array > TextField_getFontList(FlashEditInstance* editInstance) const;

	Ref< FlashTextFormat > TextField_getNewTextFormat(FlashEditInstance* editInstance) const;

	Ref< FlashTextFormat > TextField_getTextFormat_0(FlashEditInstance* editInstance) const;

	Ref< FlashTextFormat > TextField_getTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex) const;

	bool TextField_removeListener(FlashEditInstance* editInstance) const;

	void TextField_removeTextField(FlashEditInstance* editInstance) const;

	void TextField_replaceSel(FlashEditInstance* editInstance, const std::wstring& newText) const;

	void TextField_replaceText(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, const std::wstring& newText) const;

	void TextField_setNewTextFormat(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const;

	void TextField_setTextFormat_0(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const;

	void TextField_setTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, FlashTextFormat* textFormat) const;

	std::string TextField_toString(const FlashEditInstance* self) const;

	avm_number_t TextField_get_alpha(FlashEditInstance* editInstance) const;

	void TextField_set_alpha(FlashEditInstance* editInstance, avm_number_t alpha) const;

	std::wstring TextField_get_antiAliasType(FlashEditInstance* editInstance) const;

	void TextField_set_antiAliasType(FlashEditInstance* editInstance, const std::wstring& antiAliasType) const;

	Ref< ActionObject > TextField_get_autoSize(FlashEditInstance* editInstance) const;

	void TextField_set_autoSize(FlashEditInstance* editInstance, ActionObject* autoSize) const;

	bool TextField_get_background(FlashEditInstance* editInstance) const;

	void TextField_set_background(FlashEditInstance* editInstance, bool hasBackground) const;

	avm_number_t TextField_get_backgroundColor(FlashEditInstance* editInstance) const;

	void TextField_set_backgroundColor(FlashEditInstance* editInstance, avm_number_t backgroundColor) const;

	bool TextField_get_border(FlashEditInstance* editInstance) const;

	void TextField_set_border(FlashEditInstance* editInstance, bool hasBorder) const;

	avm_number_t TextField_get_borderColor(FlashEditInstance* editInstance) const;

	void TextField_set_borderColor(FlashEditInstance* editInstance, avm_number_t borderColor) const;

	avm_number_t TextField_get_bottomScroll(FlashEditInstance* editInstance) const;

	bool TextField_get_condenseWhite(FlashEditInstance* editInstance) const;

	void TextField_set_condenseWhite(FlashEditInstance* editInstance, bool condenseWhite) const;

	bool TextField_get_embedFonts(FlashEditInstance* editInstance) const;

	void TextField_set_embedFonts(FlashEditInstance* editInstance, bool embedFonts) const;

	Ref< Array > TextField_get_filters(FlashEditInstance* editInstance) const;

	void TextField_set_filters(FlashEditInstance* editInstance, Array* filters) const;

	std::wstring TextField_get_gridFitType(FlashEditInstance* editInstance) const;

	void TextField_set_gridFitType(FlashEditInstance* editInstance, const std::wstring& gridFitType) const;

	avm_number_t TextField_get_height(FlashEditInstance* editInstance) const;

	void TextField_set_height(FlashEditInstance* editInstance, avm_number_t height) const;

	avm_number_t TextField_get_highquality(FlashEditInstance* editInstance) const;

	void TextField_set_highquality(FlashEditInstance* editInstance, avm_number_t highQuality) const;

	avm_number_t TextField_get_hscroll(FlashEditInstance* editInstance) const;

	void TextField_set_hscroll(FlashEditInstance* editInstance, avm_number_t hscroll) const;

	bool TextField_get_html(FlashEditInstance* editInstance) const;

	void TextField_set_html(FlashEditInstance* editInstance, bool html) const;

	std::wstring TextField_get_htmlText(FlashEditInstance* editInstance) const;

	void TextField_set_htmlText(FlashEditInstance* editInstance, const std::wstring& htmlText) const;

	int32_t TextField_get_length(FlashEditInstance* editInstance) const;

	int32_t TextField_get_maxChars(FlashEditInstance* editInstance) const;

	void TextField_set_maxChars(FlashEditInstance* editInstance, int32_t maxChars) const;

	avm_number_t TextField_get_maxhscroll(FlashEditInstance* editInstance) const;

	int32_t TextField_get_maxscroll(FlashEditInstance* editInstance) const;

	Ref< FlashContextMenu > TextField_get_menu(FlashEditInstance* editInstance) const;

	void TextField_set_menu(FlashEditInstance* editInstance, FlashContextMenu* contextMenu) const;

	bool TextField_get_mouseWheelEnabled(FlashEditInstance* editInstance) const;

	void TextField_set_mouseWheelEnabled(FlashEditInstance* editInstance, bool mouseWheelEnabled) const;

	bool TextField_get_multiline(FlashEditInstance* editInstance) const;

	void TextField_set_multiline(FlashEditInstance* editInstance, bool multiliine) const;

	std::string TextField_get_name(FlashEditInstance* editInstance) const;

	void TextField_set_name(FlashEditInstance* editInstance, const std::string& name) const;

	Ref< FlashCharacterInstance > TextField_get_parent(FlashEditInstance* editInstance) const;

	void TextField_set_parent(FlashEditInstance* editInstance, FlashSpriteInstance* movieClip) const;

	bool TextField_get_password(FlashEditInstance* editInstance) const;

	void TextField_set_password(FlashEditInstance* editInstance, bool password) const;

	std::wstring TextField_get_quality(FlashEditInstance* editInstance) const;

	void TextField_set_quality(FlashEditInstance* editInstance, const std::wstring& quality) const;

	std::wstring TextField_get_restrict(FlashEditInstance* editInstance) const;

	void TextField_set_restrict(FlashEditInstance* editInstance, const std::wstring& restrict) const;

	avm_number_t TextField_get_rotation(FlashEditInstance* editInstance) const;

	void TextField_set_rotation(FlashEditInstance* editInstance, avm_number_t rotation) const;

	int32_t TextField_get_scroll(FlashEditInstance* editInstance) const;

	void TextField_set_scroll(FlashEditInstance* editInstance, int32_t scroll) const;

	bool TextField_get_selectable(FlashEditInstance* editInstance) const;

	void TextField_set_selectable(FlashEditInstance* editInstance, bool selectable) const;

	avm_number_t TextField_get_sharpness(FlashEditInstance* editInstance) const;

	void TextField_set_sharpness(FlashEditInstance* editInstance, avm_number_t sharpness) const;

	avm_number_t TextField_get_soundbuftime(FlashEditInstance* editInstance) const;

	void TextField_set_soundbuftime(FlashEditInstance* editInstance, avm_number_t soundbuftime) const;

	Ref< FlashStyleSheet > TextField_get_styleSheet(FlashEditInstance* editInstance) const;

	void TextField_set_styleSheet(FlashEditInstance* editInstance, FlashStyleSheet* styleSheet) const;

	bool TextField_get_tabEnabled(FlashEditInstance* editInstance) const;

	void TextField_set_tabEnabled(FlashEditInstance* editInstance, bool tabEnabled) const;

	int32_t TextField_get_tabIndex(FlashEditInstance* editInstance) const;

	void TextField_set_tabIndex(FlashEditInstance* editInstance, int32_t tabIndex) const;

	std::wstring TextField_get_target(FlashEditInstance* editInstance) const;

	std::wstring TextField_get_text(FlashEditInstance* editInstance) const;

	void TextField_set_text(FlashEditInstance* editInstance, const std::wstring& text) const;

	avm_number_t TextField_get_textColor(FlashEditInstance* editInstance) const;

	void TextField_set_textColor(FlashEditInstance* editInstance, avm_number_t textColor) const;

	avm_number_t TextField_get_textWidth(FlashEditInstance* editInstance) const;

	avm_number_t TextField_get_textHeight(FlashEditInstance* editInstance) const;

	avm_number_t TextField_get_thickness(FlashEditInstance* editInstance) const;

	void TextField_set_thickness(FlashEditInstance* editInstance, avm_number_t thickness) const;

	std::wstring TextField_get_type(FlashEditInstance* editInstance) const;

	void TextField_set_type(FlashEditInstance* editInstance, const std::wstring& type) const;

	std::wstring TextField_get_url(FlashEditInstance* editInstance) const;

	std::string TextField_get_variable(FlashEditInstance* editInstance) const;

	void TextField_set_variable(FlashEditInstance* editInstance, const std::string& url) const;

	bool TextField_get_visible(FlashEditInstance* editInstance) const;

	void TextField_set_visible(FlashEditInstance* editInstance, bool visible) const;

	avm_number_t TextField_get_width(FlashEditInstance* editInstance) const;

	void TextField_set_width(FlashEditInstance* editInstance, avm_number_t width) const;

	bool TextField_get_wordWrap(FlashEditInstance* editInstance) const;

	void TextField_set_wordWrap(FlashEditInstance* editInstance, bool wordWrap) const;

	avm_number_t TextField_get_x(FlashEditInstance* editInstance) const;

	void TextField_set_x(FlashEditInstance* editInstance, avm_number_t x) const;

	avm_number_t TextField_get_xmouse(FlashEditInstance* editInstance) const;

	avm_number_t TextField_get_xscale(FlashEditInstance* editInstance) const;

	void TextField_set_xscale(FlashEditInstance* editInstance, avm_number_t xscale) const;

	avm_number_t TextField_get_y(FlashEditInstance* editInstance) const;

	void TextField_set_y(FlashEditInstance* editInstance, avm_number_t y) const;

	avm_number_t TextField_get_ymouse(FlashEditInstance* editInstance) const;

	avm_number_t TextField_get_yscale(FlashEditInstance* editInstance) const;

	void TextField_set_yscale(FlashEditInstance* editInstance, avm_number_t yscale) const;
};

	}
}

#endif	// traktor_flash_AsTextField_H
