#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField", AsTextField, ActionClass)

AsTextField::AsTextField()
:	ActionClass("TextField")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("StyleSheet", ActionValue(new AsTextField_StyleSheet()));
	prototype->setMember("addListener", ActionValue(createNativeFunction(this, &AsTextField::TextField_addListener)));
	prototype->setMember("getDepth", ActionValue(createNativeFunction(this, &AsTextField::TextField_getDepth)));
	prototype->setMember("getFontList", ActionValue(createNativeFunction(this, &AsTextField::TextField_getFontList)));
	prototype->setMember("getNewTextFormat", ActionValue(createNativeFunction(this, &AsTextField::TextField_getNewTextFormat)));
	prototype->setMember("getTextFormat", ActionValue(
		createPolymorphicFunction(
			createNativeFunction(this, &AsTextField::TextField_getTextFormat_0),
			0,
			createNativeFunction(this, &AsTextField::TextField_getTextFormat_2)
		)
	));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(this, &AsTextField::TextField_removeListener)));
	prototype->setMember("removeTextField", ActionValue(createNativeFunction(this, &AsTextField::TextField_removeTextField)));
	prototype->setMember("replaceSel", ActionValue(createNativeFunction(this, &AsTextField::TextField_replaceSel)));
	prototype->setMember("replaceText", ActionValue(createNativeFunction(this, &AsTextField::TextField_replaceText)));
	prototype->setMember("setNewTextFormat", ActionValue(createNativeFunction(this, &AsTextField::TextField_setNewTextFormat)));
	prototype->setMember("setTextFormat", ActionValue(
		createPolymorphicFunction(
			createNativeFunction(this, &AsTextField::TextField_setTextFormat_0),
			0,
			createNativeFunction(this, &AsTextField::TextField_setTextFormat_2)
		)
	));

	prototype->addProperty("_alpha", createNativeFunction(this, &AsTextField::TextField_get_alpha), createNativeFunction(this, &AsTextField::TextField_set_alpha));
	prototype->addProperty("antiAliasType", createNativeFunction(this, &AsTextField::TextField_get_antiAliasType), createNativeFunction(this, &AsTextField::TextField_set_antiAliasType));
	prototype->addProperty("autoSize", createNativeFunction(this, &AsTextField::TextField_get_autoSize), createNativeFunction(this, &AsTextField::TextField_set_autoSize));
	prototype->addProperty("background", createNativeFunction(this, &AsTextField::TextField_get_background), createNativeFunction(this, &AsTextField::TextField_set_background));
	prototype->addProperty("backgroundColor", createNativeFunction(this, &AsTextField::TextField_get_backgroundColor), createNativeFunction(this, &AsTextField::TextField_set_backgroundColor));
	prototype->addProperty("border", createNativeFunction(this, &AsTextField::TextField_get_border), createNativeFunction(this, &AsTextField::TextField_set_border));
	prototype->addProperty("borderColor", createNativeFunction(this, &AsTextField::TextField_get_borderColor), createNativeFunction(this, &AsTextField::TextField_set_borderColor));
	prototype->addProperty("bottomScroll", createNativeFunction(this, &AsTextField::TextField_get_bottomScroll), 0);
	prototype->addProperty("condenseWhite", createNativeFunction(this, &AsTextField::TextField_get_condenseWhite), createNativeFunction(this, &AsTextField::TextField_set_condenseWhite));
	prototype->addProperty("embedFonts", createNativeFunction(this, &AsTextField::TextField_get_embedFonts), createNativeFunction(this, &AsTextField::TextField_set_embedFonts));
	prototype->addProperty("filters", createNativeFunction(this, &AsTextField::TextField_get_filters), createNativeFunction(this, &AsTextField::TextField_set_filters));
	prototype->addProperty("gridFitType", createNativeFunction(this, &AsTextField::TextField_get_gridFitType), createNativeFunction(this, &AsTextField::TextField_set_gridFitType));
	prototype->addProperty("_height", createNativeFunction(this, &AsTextField::TextField_get_height), createNativeFunction(this, &AsTextField::TextField_set_height));
	prototype->addProperty("_highquality", createNativeFunction(this, &AsTextField::TextField_get_highquality), createNativeFunction(this, &AsTextField::TextField_set_highquality));
	prototype->addProperty("hscroll", createNativeFunction(this, &AsTextField::TextField_get_hscroll), createNativeFunction(this, &AsTextField::TextField_set_hscroll));
	prototype->addProperty("html", createNativeFunction(this, &AsTextField::TextField_get_html), createNativeFunction(this, &AsTextField::TextField_set_html));
	prototype->addProperty("htmlText", createNativeFunction(this, &AsTextField::TextField_get_htmlText), createNativeFunction(this, &AsTextField::TextField_set_htmlText));
	prototype->addProperty("length", createNativeFunction(this, &AsTextField::TextField_get_length), 0);
	prototype->addProperty("maxChars", createNativeFunction(this, &AsTextField::TextField_get_maxChars), createNativeFunction(this, &AsTextField::TextField_set_maxChars));
	prototype->addProperty("maxhscroll", createNativeFunction(this, &AsTextField::TextField_get_maxhscroll), 0);
	prototype->addProperty("maxscroll", createNativeFunction(this, &AsTextField::TextField_get_maxscroll), 0);
	//prototype->addProperty("menu", createNativeFunction(this, &AsTextField::TextField_get_menu), createNativeFunction(this, &AsTextField::TextField_set_menu));
	prototype->addProperty("mouseWheelEnabled", createNativeFunction(this, &AsTextField::TextField_get_mouseWheelEnabled), createNativeFunction(this, &AsTextField::TextField_set_mouseWheelEnabled));
	prototype->addProperty("multiline", createNativeFunction(this, &AsTextField::TextField_get_multiline), createNativeFunction(this, &AsTextField::TextField_set_multiline));
	prototype->addProperty("_name", createNativeFunction(this, &AsTextField::TextField_get_name), createNativeFunction(this, &AsTextField::TextField_set_name));
	prototype->addProperty("_parent", createNativeFunction(this, &AsTextField::TextField_get_parent), createNativeFunction(this, &AsTextField::TextField_set_parent));
	prototype->addProperty("password", createNativeFunction(this, &AsTextField::TextField_get_password), createNativeFunction(this, &AsTextField::TextField_set_password));
	prototype->addProperty("_quality", createNativeFunction(this, &AsTextField::TextField_get_quality), createNativeFunction(this, &AsTextField::TextField_set_quality));
	prototype->addProperty("restrict", createNativeFunction(this, &AsTextField::TextField_get_restrict), createNativeFunction(this, &AsTextField::TextField_set_restrict));
	prototype->addProperty("_rotation", createNativeFunction(this, &AsTextField::TextField_get_rotation), createNativeFunction(this, &AsTextField::TextField_set_rotation));
	prototype->addProperty("scroll", createNativeFunction(this, &AsTextField::TextField_get_scroll), createNativeFunction(this, &AsTextField::TextField_set_scroll));
	prototype->addProperty("selectable", createNativeFunction(this, &AsTextField::TextField_get_selectable), createNativeFunction(this, &AsTextField::TextField_set_selectable));
	prototype->addProperty("sharpness", createNativeFunction(this, &AsTextField::TextField_get_sharpness), createNativeFunction(this, &AsTextField::TextField_set_sharpness));
	prototype->addProperty("_soundbuftime", createNativeFunction(this, &AsTextField::TextField_get_soundbuftime), createNativeFunction(this, &AsTextField::TextField_set_soundbuftime));
	//prototype->addProperty("styleSheet", createNativeFunction(this, &AsTextField::TextField_get_styleSheet), createNativeFunction(this, &AsTextField::TextField_set_styleSheet));
	prototype->addProperty("tabEnabled", createNativeFunction(this, &AsTextField::TextField_get_tabEnabled), createNativeFunction(this, &AsTextField::TextField_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(this, &AsTextField::TextField_get_tabIndex), createNativeFunction(this, &AsTextField::TextField_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(this, &AsTextField::TextField_get_target), 0);
	prototype->addProperty("text", createNativeFunction(this, &AsTextField::TextField_get_text), createNativeFunction(this, &AsTextField::TextField_set_text));
	prototype->addProperty("textColor", createNativeFunction(this, &AsTextField::TextField_get_textColor), createNativeFunction(this, &AsTextField::TextField_set_textColor));
	prototype->addProperty("textWidth", createNativeFunction(this, &AsTextField::TextField_get_textWidth), 0);
	prototype->addProperty("textHeight", createNativeFunction(this, &AsTextField::TextField_get_textHeight), 0);
	prototype->addProperty("thickness", createNativeFunction(this, &AsTextField::TextField_get_thickness), createNativeFunction(this, &AsTextField::TextField_set_thickness));
	prototype->addProperty("type", createNativeFunction(this, &AsTextField::TextField_get_type), createNativeFunction(this, &AsTextField::TextField_set_type));
	prototype->addProperty("_url", createNativeFunction(this, &AsTextField::TextField_get_url), 0);
	prototype->addProperty("variable", createNativeFunction(this, &AsTextField::TextField_get_variable), createNativeFunction(this, &AsTextField::TextField_set_variable));
	prototype->addProperty("_visible", createNativeFunction(this, &AsTextField::TextField_get_visible), createNativeFunction(this, &AsTextField::TextField_set_visible));
	prototype->addProperty("_width", createNativeFunction(this, &AsTextField::TextField_get_width), createNativeFunction(this, &AsTextField::TextField_set_width));
	prototype->addProperty("wordWrap", createNativeFunction(this, &AsTextField::TextField_get_wordWrap), createNativeFunction(this, &AsTextField::TextField_set_wordWrap));
	prototype->addProperty("_x", createNativeFunction(this, &AsTextField::TextField_get_x), createNativeFunction(this, &AsTextField::TextField_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(this, &AsTextField::TextField_get_xmouse), 0);
	prototype->addProperty("_xscale", createNativeFunction(this, &AsTextField::TextField_get_xscale), createNativeFunction(this, &AsTextField::TextField_set_xscale));
	prototype->addProperty("_y", createNativeFunction(this, &AsTextField::TextField_get_y), createNativeFunction(this, &AsTextField::TextField_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(this, &AsTextField::TextField_get_ymouse), 0);
	prototype->addProperty("_yscale", createNativeFunction(this, &AsTextField::TextField_get_yscale), createNativeFunction(this, &AsTextField::TextField_set_yscale));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsTextField::alloc(ActionContext* context)
{
	return 0;
}

void AsTextField::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
}

void AsTextField::TextField_addListener(FlashEditInstance* editInstance, ActionObject* listener) const
{
}

avm_number_t AsTextField::TextField_getDepth(FlashEditInstance* editInstance) const
{
	return 0;
}

Ref< Array > AsTextField::TextField_getFontList(FlashEditInstance* editInstance) const
{
	return 0;
}

Ref< FlashTextFormat > AsTextField::TextField_getNewTextFormat(FlashEditInstance* editInstance) const
{
	return 0;
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_0(FlashEditInstance* editInstance) const
{
	return 0;
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex) const
{
	return 0;
}

bool AsTextField::TextField_removeListener(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_removeTextField(FlashEditInstance* editInstance) const
{
}

void AsTextField::TextField_replaceSel(FlashEditInstance* editInstance, const std::wstring& newText) const
{
}

void AsTextField::TextField_replaceText(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, const std::wstring& newText) const
{
}

void AsTextField::TextField_setNewTextFormat(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const
{
}

void AsTextField::TextField_setTextFormat_0(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const
{
}

void AsTextField::TextField_setTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, FlashTextFormat* textFormat) const
{
}

avm_number_t AsTextField::TextField_get_alpha(FlashEditInstance* editInstance) const
{
	return 100.0f;
}

void AsTextField::TextField_set_alpha(FlashEditInstance* editInstance, avm_number_t alpha) const
{
}

std::wstring AsTextField::TextField_get_antiAliasType(FlashEditInstance* editInstance) const
{
	return L"normal";
}

void AsTextField::TextField_set_antiAliasType(FlashEditInstance* editInstance, const std::wstring& antiAliasType) const
{
}

Ref< ActionObject > AsTextField::TextField_get_autoSize(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_autoSize(FlashEditInstance* editInstance, ActionObject* autoSize) const
{
}

bool AsTextField::TextField_get_background(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_background(FlashEditInstance* editInstance, bool hasBackground) const
{
}

avm_number_t AsTextField::TextField_get_backgroundColor(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_backgroundColor(FlashEditInstance* editInstance, avm_number_t backgroundColor) const
{
}

bool AsTextField::TextField_get_border(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_border(FlashEditInstance* editInstance, bool hasBorder) const
{
}

avm_number_t AsTextField::TextField_get_borderColor(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_borderColor(FlashEditInstance* editInstance, avm_number_t borderColor) const
{
}

avm_number_t AsTextField::TextField_get_bottomScroll(FlashEditInstance* editInstance) const
{
	return 0;
}

bool AsTextField::TextField_get_condenseWhite(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_condenseWhite(FlashEditInstance* editInstance, bool condenseWhite) const
{
}

bool AsTextField::TextField_get_embedFonts(FlashEditInstance* editInstance) const
{
	return true;
}

void AsTextField::TextField_set_embedFonts(FlashEditInstance* editInstance, bool embedFonts) const
{
}

Ref< Array > AsTextField::TextField_get_filters(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_filters(FlashEditInstance* editInstance, Array* filters) const
{
}

std::wstring AsTextField::TextField_get_gridFitType(FlashEditInstance* editInstance) const
{
	return L"none";
}

void AsTextField::TextField_set_gridFitType(FlashEditInstance* editInstance, const std::wstring& gridFitType) const
{
}

avm_number_t AsTextField::TextField_get_height(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_height(FlashEditInstance* editInstance, avm_number_t height) const
{
}

avm_number_t AsTextField::TextField_get_highquality(FlashEditInstance* editInstance) const
{
	return 2;
}

void AsTextField::TextField_set_highquality(FlashEditInstance* editInstance, avm_number_t highQuality) const
{
}

avm_number_t AsTextField::TextField_get_hscroll(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_hscroll(FlashEditInstance* editInstance, avm_number_t hscroll) const
{
}

bool AsTextField::TextField_get_html(FlashEditInstance* editInstance) const
{
	return editInstance->getEdit()->renderHtml();
}

void AsTextField::TextField_set_html(FlashEditInstance* editInstance, bool html) const
{
}

std::wstring AsTextField::TextField_get_htmlText(FlashEditInstance* editInstance) const
{
	return L"";
}

void AsTextField::TextField_set_htmlText(FlashEditInstance* editInstance, const std::wstring& htmlText) const
{
}

int32_t AsTextField::TextField_get_length(FlashEditInstance* editInstance) const
{
	return int32_t(editInstance->getConcatedText().length());
}

int32_t AsTextField::TextField_get_maxChars(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_maxChars(FlashEditInstance* editInstance, int32_t maxChars) const
{
}

avm_number_t AsTextField::TextField_get_maxhscroll(FlashEditInstance* editInstance) const
{
	return 0;
}

avm_number_t AsTextField::TextField_get_maxscroll(FlashEditInstance* editInstance) const
{
	return 0;
}

Ref< FlashContextMenu > AsTextField::TextField_get_menu(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_menu(FlashEditInstance* editInstance, FlashContextMenu* contextMenu) const
{
}

bool AsTextField::TextField_get_mouseWheelEnabled(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_mouseWheelEnabled(FlashEditInstance* editInstance, bool mouseWheelEnabled) const
{
}

bool AsTextField::TextField_get_multiline(FlashEditInstance* editInstance) const
{
	return true;
}

void AsTextField::TextField_set_multiline(FlashEditInstance* editInstance, bool multiliine) const
{
}

std::string AsTextField::TextField_get_name(FlashEditInstance* editInstance) const
{
	return editInstance->getName();
}

void AsTextField::TextField_set_name(FlashEditInstance* editInstance, const std::string& name) const
{
	editInstance->setName(name);
}

Ref< FlashSpriteInstance > AsTextField::TextField_get_parent(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_parent(FlashEditInstance* editInstance, FlashSpriteInstance* movieClip) const
{
}

bool AsTextField::TextField_get_password(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_password(FlashEditInstance* editInstance, bool password) const
{
}

std::wstring AsTextField::TextField_get_quality(FlashEditInstance* editInstance) const
{
	return L"BEST";
}

void AsTextField::TextField_set_quality(FlashEditInstance* editInstance, const std::wstring& quality) const
{
}

std::wstring AsTextField::TextField_get_restrict(FlashEditInstance* editInstance) const
{
	return L"";
}

void AsTextField::TextField_set_restrict(FlashEditInstance* editInstance, const std::wstring& restrict) const
{
}

avm_number_t AsTextField::TextField_get_rotation(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_rotation(FlashEditInstance* editInstance, avm_number_t rotation) const
{
}

avm_number_t AsTextField::TextField_get_scroll(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_scroll(FlashEditInstance* editInstance, avm_number_t scroll) const
{
}

bool AsTextField::TextField_get_selectable(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_selectable(FlashEditInstance* editInstance, bool selectable) const
{
}

avm_number_t AsTextField::TextField_get_sharpness(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_sharpness(FlashEditInstance* editInstance, avm_number_t sharpness) const
{
}

avm_number_t AsTextField::TextField_get_soundbuftime(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_soundbuftime(FlashEditInstance* editInstance, avm_number_t soundbuftime) const
{
}

Ref< FlashStyleSheet > AsTextField::TextField_get_styleSheet(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_styleSheet(FlashEditInstance* editInstance, FlashStyleSheet* styleSheet) const
{
}

bool AsTextField::TextField_get_tabEnabled(FlashEditInstance* editInstance) const
{
	return false;
}

void AsTextField::TextField_set_tabEnabled(FlashEditInstance* editInstance, bool tabEnabled) const
{
}

int32_t AsTextField::TextField_get_tabIndex(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_tabIndex(FlashEditInstance* editInstance, int32_t tabIndex) const
{
}

std::wstring AsTextField::TextField_get_target(FlashEditInstance* editInstance) const
{
	return L"";
}

std::wstring AsTextField::TextField_get_text(FlashEditInstance* editInstance) const
{
	return editInstance->getConcatedText();
}

void AsTextField::TextField_set_text(FlashEditInstance* editInstance, const std::wstring& text) const
{
	editInstance->parseText(text);
}

avm_number_t AsTextField::TextField_get_textColor(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_textColor(FlashEditInstance* editInstance, avm_number_t textColor) const
{
}

avm_number_t AsTextField::TextField_get_textWidth(FlashEditInstance* editInstance) const
{
	float width, height;
	if (editInstance->getTextExtents(width, height))
		return avm_number_t(width);
	else
		return avm_number_t(0);
}

avm_number_t AsTextField::TextField_get_textHeight(FlashEditInstance* editInstance) const
{
	float width, height;
	if (editInstance->getTextExtents(width, height))
		return avm_number_t(height);
	else
		return avm_number_t(0);
}

avm_number_t AsTextField::TextField_get_thickness(FlashEditInstance* editInstance) const
{
	return 0;
}

void AsTextField::TextField_set_thickness(FlashEditInstance* editInstance, avm_number_t thickness) const
{
}

std::wstring AsTextField::TextField_get_type(FlashEditInstance* editInstance) const
{
	return L"input";
}

void AsTextField::TextField_set_type(FlashEditInstance* editInstance, const std::wstring& type) const
{
}

std::wstring AsTextField::TextField_get_url(FlashEditInstance* editInstance) const
{
	return L"";
}

std::string AsTextField::TextField_get_variable(FlashEditInstance* editInstance) const
{
	return "";
}

void AsTextField::TextField_set_variable(FlashEditInstance* editInstance, const std::string& url) const
{
}

bool AsTextField::TextField_get_visible(FlashEditInstance* editInstance) const
{
	return true;
}

void AsTextField::TextField_set_visible(FlashEditInstance* editInstance, bool visible) const
{
}

avm_number_t AsTextField::TextField_get_width(FlashEditInstance* editInstance) const
{
	SwfRect bounds = editInstance->getBounds();
	return bounds.max.x - bounds.min.x;
}

void AsTextField::TextField_set_width(FlashEditInstance* editInstance, avm_number_t width) const
{
}

bool AsTextField::TextField_get_wordWrap(FlashEditInstance* editInstance) const
{
	return editInstance->getEdit()->wordWrap();
}

void AsTextField::TextField_set_wordWrap(FlashEditInstance* editInstance, bool wordWrap) const
{
}

avm_number_t AsTextField::TextField_get_x(FlashEditInstance* editInstance) const
{
	SwfRect bounds = editInstance->getBounds();
	return bounds.min.x;
}

void AsTextField::TextField_set_x(FlashEditInstance* editInstance, avm_number_t x) const
{
}

avm_number_t AsTextField::TextField_get_xmouse(FlashEditInstance* editInstance) const
{
	return 0;
}

avm_number_t AsTextField::TextField_get_xscale(FlashEditInstance* editInstance) const
{
	return 1;
}

void AsTextField::TextField_set_xscale(FlashEditInstance* editInstance, avm_number_t xscale) const
{
}

avm_number_t AsTextField::TextField_get_y(FlashEditInstance* editInstance) const
{
	SwfRect bounds = editInstance->getBounds();
	return bounds.min.y;
}

void AsTextField::TextField_set_y(FlashEditInstance* editInstance, avm_number_t y) const
{
}

avm_number_t AsTextField::TextField_get_ymouse(FlashEditInstance* editInstance) const
{
	return 0;
}

avm_number_t AsTextField::TextField_get_yscale(FlashEditInstance* editInstance) const
{
	return 1;
}

void AsTextField::TextField_set_yscale(FlashEditInstance* editInstance, avm_number_t yscale) const
{
}

	}
}
