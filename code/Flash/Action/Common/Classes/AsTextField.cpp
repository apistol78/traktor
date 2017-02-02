#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsTextField.h"
#include "Flash/Action/Common/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Common/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField", AsTextField, ActionClass)

AsTextField::AsTextField(ActionContext* context)
:	ActionClass(context, "TextField")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("StyleSheet", ActionValue(new AsTextField_StyleSheet(context)));
	prototype->setMember("addListener", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_addListener)));
	prototype->setMember("getDepth", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getDepth)));
	prototype->setMember("getFontList", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getFontList)));
	prototype->setMember("getNewTextFormat", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getNewTextFormat)));
	prototype->setMember("getTextFormat", ActionValue(
		createPolymorphicFunction(
			context,
			createNativeFunction(context, this, &AsTextField::TextField_getTextFormat_0),
			0,
			createNativeFunction(context, this, &AsTextField::TextField_getTextFormat_2)
		)
	));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_removeListener)));
	prototype->setMember("removeTextField", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_removeTextField)));
	prototype->setMember("replaceSel", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_replaceSel)));
	prototype->setMember("replaceText", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_replaceText)));
	prototype->setMember("setNewTextFormat", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_setNewTextFormat)));
	prototype->setMember("setTextFormat", ActionValue(
		createPolymorphicFunction(
			context,
			createNativeFunction(context, this, &AsTextField::TextField_setTextFormat_0),
			0,
			createNativeFunction(context, this, &AsTextField::TextField_setTextFormat_2)
		)
	));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_toString)));

	prototype->addProperty("_alpha", createNativeFunction(context, this, &AsTextField::TextField_get_alpha), createNativeFunction(context, this, &AsTextField::TextField_set_alpha));
	prototype->addProperty("antiAliasType", createNativeFunction(context, this, &AsTextField::TextField_get_antiAliasType), createNativeFunction(context, this, &AsTextField::TextField_set_antiAliasType));
	prototype->addProperty("autoSize", createNativeFunction(context, this, &AsTextField::TextField_get_autoSize), createNativeFunction(context, this, &AsTextField::TextField_set_autoSize));
	prototype->addProperty("background", createNativeFunction(context, this, &AsTextField::TextField_get_background), createNativeFunction(context, this, &AsTextField::TextField_set_background));
	prototype->addProperty("backgroundColor", createNativeFunction(context, this, &AsTextField::TextField_get_backgroundColor), createNativeFunction(context, this, &AsTextField::TextField_set_backgroundColor));
	prototype->addProperty("border", createNativeFunction(context, this, &AsTextField::TextField_get_border), createNativeFunction(context, this, &AsTextField::TextField_set_border));
	prototype->addProperty("borderColor", createNativeFunction(context, this, &AsTextField::TextField_get_borderColor), createNativeFunction(context, this, &AsTextField::TextField_set_borderColor));
	prototype->addProperty("bottomScroll", createNativeFunction(context, this, &AsTextField::TextField_get_bottomScroll), 0);
	prototype->addProperty("condenseWhite", createNativeFunction(context, this, &AsTextField::TextField_get_condenseWhite), createNativeFunction(context, this, &AsTextField::TextField_set_condenseWhite));
	prototype->addProperty("embedFonts", createNativeFunction(context, this, &AsTextField::TextField_get_embedFonts), createNativeFunction(context, this, &AsTextField::TextField_set_embedFonts));
	prototype->addProperty("filters", createNativeFunction(context, this, &AsTextField::TextField_get_filters), createNativeFunction(context, this, &AsTextField::TextField_set_filters));
	prototype->addProperty("gridFitType", createNativeFunction(context, this, &AsTextField::TextField_get_gridFitType), createNativeFunction(context, this, &AsTextField::TextField_set_gridFitType));
	prototype->addProperty("_height", createNativeFunction(context, this, &AsTextField::TextField_get_height), createNativeFunction(context, this, &AsTextField::TextField_set_height));
	prototype->addProperty("_highquality", createNativeFunction(context, this, &AsTextField::TextField_get_highquality), createNativeFunction(context, this, &AsTextField::TextField_set_highquality));
	prototype->addProperty("hscroll", createNativeFunction(context, this, &AsTextField::TextField_get_hscroll), createNativeFunction(context, this, &AsTextField::TextField_set_hscroll));
	prototype->addProperty("html", createNativeFunction(context, this, &AsTextField::TextField_get_html), createNativeFunction(context, this, &AsTextField::TextField_set_html));
	prototype->addProperty("htmlText", createNativeFunction(context, this, &AsTextField::TextField_get_htmlText), createNativeFunction(context, this, &AsTextField::TextField_set_htmlText));
	prototype->addProperty("length", createNativeFunction(context, this, &AsTextField::TextField_get_length), 0);
	prototype->addProperty("maxChars", createNativeFunction(context, this, &AsTextField::TextField_get_maxChars), createNativeFunction(context, this, &AsTextField::TextField_set_maxChars));
	prototype->addProperty("maxhscroll", createNativeFunction(context, this, &AsTextField::TextField_get_maxhscroll), 0);
	prototype->addProperty("maxscroll", createNativeFunction(context, this, &AsTextField::TextField_get_maxscroll), 0);
	//prototype->addProperty("menu", createNativeFunction(context, this, &AsTextField::TextField_get_menu), createNativeFunction(context, this, &AsTextField::TextField_set_menu));
	prototype->addProperty("mouseWheelEnabled", createNativeFunction(context, this, &AsTextField::TextField_get_mouseWheelEnabled), createNativeFunction(context, this, &AsTextField::TextField_set_mouseWheelEnabled));
	prototype->addProperty("multiline", createNativeFunction(context, this, &AsTextField::TextField_get_multiline), createNativeFunction(context, this, &AsTextField::TextField_set_multiline));
	prototype->addProperty("_name", createNativeFunction(context, this, &AsTextField::TextField_get_name), createNativeFunction(context, this, &AsTextField::TextField_set_name));
	prototype->addProperty("_parent", createNativeFunction(context, this, &AsTextField::TextField_get_parent), createNativeFunction(context, this, &AsTextField::TextField_set_parent));
	prototype->addProperty("password", createNativeFunction(context, this, &AsTextField::TextField_get_password), createNativeFunction(context, this, &AsTextField::TextField_set_password));
	prototype->addProperty("_quality", createNativeFunction(context, this, &AsTextField::TextField_get_quality), createNativeFunction(context, this, &AsTextField::TextField_set_quality));
	prototype->addProperty("restrict", createNativeFunction(context, this, &AsTextField::TextField_get_restrict), createNativeFunction(context, this, &AsTextField::TextField_set_restrict));
	prototype->addProperty("_rotation", createNativeFunction(context, this, &AsTextField::TextField_get_rotation), createNativeFunction(context, this, &AsTextField::TextField_set_rotation));
	prototype->addProperty("scroll", createNativeFunction(context, this, &AsTextField::TextField_get_scroll), createNativeFunction(context, this, &AsTextField::TextField_set_scroll));
	prototype->addProperty("selectable", createNativeFunction(context, this, &AsTextField::TextField_get_selectable), createNativeFunction(context, this, &AsTextField::TextField_set_selectable));
	prototype->addProperty("sharpness", createNativeFunction(context, this, &AsTextField::TextField_get_sharpness), createNativeFunction(context, this, &AsTextField::TextField_set_sharpness));
	prototype->addProperty("_soundbuftime", createNativeFunction(context, this, &AsTextField::TextField_get_soundbuftime), createNativeFunction(context, this, &AsTextField::TextField_set_soundbuftime));
	//prototype->addProperty("styleSheet", createNativeFunction(context, this, &AsTextField::TextField_get_styleSheet), createNativeFunction(context, this, &AsTextField::TextField_set_styleSheet));
	prototype->addProperty("tabEnabled", createNativeFunction(context, this, &AsTextField::TextField_get_tabEnabled), createNativeFunction(context, this, &AsTextField::TextField_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(context, this, &AsTextField::TextField_get_tabIndex), createNativeFunction(context, this, &AsTextField::TextField_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(context, this, &AsTextField::TextField_get_target), 0);
	prototype->addProperty("text", createNativeFunction(context, this, &AsTextField::TextField_get_text), createNativeFunction(context, this, &AsTextField::TextField_set_text));
	prototype->addProperty("textColor", createNativeFunction(context, this, &AsTextField::TextField_get_textColor), createNativeFunction(context, this, &AsTextField::TextField_set_textColor));
	prototype->addProperty("textWidth", createNativeFunction(context, this, &AsTextField::TextField_get_textWidth), 0);
	prototype->addProperty("textHeight", createNativeFunction(context, this, &AsTextField::TextField_get_textHeight), 0);
	prototype->addProperty("thickness", createNativeFunction(context, this, &AsTextField::TextField_get_thickness), createNativeFunction(context, this, &AsTextField::TextField_set_thickness));
	prototype->addProperty("type", createNativeFunction(context, this, &AsTextField::TextField_get_type), createNativeFunction(context, this, &AsTextField::TextField_set_type));
	prototype->addProperty("_url", createNativeFunction(context, this, &AsTextField::TextField_get_url), 0);
	prototype->addProperty("variable", createNativeFunction(context, this, &AsTextField::TextField_get_variable), createNativeFunction(context, this, &AsTextField::TextField_set_variable));
	prototype->addProperty("_visible", createNativeFunction(context, this, &AsTextField::TextField_get_visible), createNativeFunction(context, this, &AsTextField::TextField_set_visible));
	prototype->addProperty("_width", createNativeFunction(context, this, &AsTextField::TextField_get_width), createNativeFunction(context, this, &AsTextField::TextField_set_width));
	prototype->addProperty("wordWrap", createNativeFunction(context, this, &AsTextField::TextField_get_wordWrap), createNativeFunction(context, this, &AsTextField::TextField_set_wordWrap));
	prototype->addProperty("_x", createNativeFunction(context, this, &AsTextField::TextField_get_x), createNativeFunction(context, this, &AsTextField::TextField_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(context, this, &AsTextField::TextField_get_xmouse), 0);
	prototype->addProperty("_xscale", createNativeFunction(context, this, &AsTextField::TextField_get_xscale), createNativeFunction(context, this, &AsTextField::TextField_set_xscale));
	prototype->addProperty("_y", createNativeFunction(context, this, &AsTextField::TextField_get_y), createNativeFunction(context, this, &AsTextField::TextField_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(context, this, &AsTextField::TextField_get_ymouse), 0);
	prototype->addProperty("_yscale", createNativeFunction(context, this, &AsTextField::TextField_get_yscale), createNativeFunction(context, this, &AsTextField::TextField_set_yscale));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsTextField::initialize(ActionObject* self)
{
}

void AsTextField::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsTextField::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsTextField::TextField_addListener(FlashEditInstance* self, ActionObject* listener) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::addListener not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_getDepth(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::getDepth not implemented" << Endl;
	)
	return 0;
}

Ref< Array > AsTextField::TextField_getFontList(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::getFontList not implemented" << Endl;
	)
	return 0;
}

Ref< FlashTextFormat > AsTextField::TextField_getNewTextFormat(FlashEditInstance* self) const
{
	return self->getTextFormat();
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_0(FlashEditInstance* self) const
{
	return self->getTextFormat();
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_2(FlashEditInstance* self, int32_t beginIndex, int32_t endIndex) const
{
	return self->getTextFormat(beginIndex, endIndex);
}

bool AsTextField::TextField_removeListener(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::removeListener not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_removeTextField(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::removeTextField not implemented" << Endl;
	)
}

void AsTextField::TextField_replaceSel(FlashEditInstance* self, const std::wstring& newText) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::replaceSel not implemented" << Endl;
	)
}

void AsTextField::TextField_replaceText(FlashEditInstance* self, int32_t beginIndex, int32_t endIndex, const std::wstring& newText) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::replaceText not implemented" << Endl;
	)
}

void AsTextField::TextField_setNewTextFormat(FlashEditInstance* self, FlashTextFormat* textFormat) const
{
	self->setTextFormat(textFormat);
}

void AsTextField::TextField_setTextFormat_0(FlashEditInstance* self, FlashTextFormat* textFormat) const
{
	self->setTextFormat(textFormat);
}

void AsTextField::TextField_setTextFormat_2(FlashEditInstance* self, int32_t beginIndex, int32_t endIndex, FlashTextFormat* textFormat) const
{
	self->setTextFormat(textFormat, beginIndex, endIndex);
}

std::string AsTextField::TextField_toString(const FlashEditInstance* self) const
{
	std::string target = self->getTarget();
	return "_level0" + replaceAll(target, '/', '.');
}

avm_number_t AsTextField::TextField_get_alpha(FlashEditInstance* self) const
{
	const ColorTransform& colorTransform = self->getColorTransform();
	return colorTransform.mul.getAlpha() * 100.0f;
}

void AsTextField::TextField_set_alpha(FlashEditInstance* self, avm_number_t alpha) const
{
	ColorTransform colorTransform = self->getColorTransform();
	colorTransform.mul.setAlpha(Scalar(alpha / 100.0f));
	self->setColorTransform(colorTransform);
}

std::wstring AsTextField::TextField_get_antiAliasType(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_antiAliasType not implemented" << Endl;
	)
	return L"normal";
}

void AsTextField::TextField_set_antiAliasType(FlashEditInstance* self, const std::wstring& antiAliasType) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_antiAliasType not implemented" << Endl;
	)
}

Ref< ActionObject > AsTextField::TextField_get_autoSize(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_autoSize not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_autoSize(FlashEditInstance* self, ActionObject* autoSize) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_autoSize not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_background(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_background not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_background(FlashEditInstance* self, bool hasBackground) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_background not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_backgroundColor(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_backgroundColor not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_backgroundColor(FlashEditInstance* self, avm_number_t backgroundColor) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_backgroundColor not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_border(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_border not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_border(FlashEditInstance* self, bool hasBorder) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_border not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_borderColor(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_borderColor not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_borderColor(FlashEditInstance* self, avm_number_t borderColor) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_borderColor not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_bottomScroll(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_bottomScroll not implemented" << Endl;
	)
	return 0;
}

bool AsTextField::TextField_get_condenseWhite(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_condenseWhite not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_condenseWhite(FlashEditInstance* self, bool condenseWhite) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_condenseWhite not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_embedFonts(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_embedFonts not implemented" << Endl;
	)
	return true;
}

void AsTextField::TextField_set_embedFonts(FlashEditInstance* self, bool embedFonts) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_embedFonts not implemented" << Endl;
	)
}

Ref< Array > AsTextField::TextField_get_filters(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_filters not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_filters(FlashEditInstance* self, Array* filters) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_filters not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_gridFitType(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_gridFitType not implemented" << Endl;
	)
	return L"none";
}

void AsTextField::TextField_set_gridFitType(FlashEditInstance* self, const std::wstring& gridFitType) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_gridFitType not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_height(FlashEditInstance* self) const
{
	return self->getHeight();
}

void AsTextField::TextField_set_height(FlashEditInstance* self, avm_number_t height) const
{
	self->setHeight(height);
}

avm_number_t AsTextField::TextField_get_highquality(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_highquality not implemented" << Endl;
	)
	return 2;
}

void AsTextField::TextField_set_highquality(FlashEditInstance* self, avm_number_t highQuality) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_highquality not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_hscroll(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_hscroll not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_hscroll(FlashEditInstance* self, avm_number_t hscroll) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_hscroll not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_html(FlashEditInstance* self) const
{
	return self->getEdit()->renderHtml();
}

void AsTextField::TextField_set_html(FlashEditInstance* self, bool html) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_html not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_htmlText(FlashEditInstance* self) const
{
	return self->getHtmlText();
}

void AsTextField::TextField_set_htmlText(FlashEditInstance* self, const std::wstring& htmlText) const
{
	self->parseHtml(htmlText);
}

int32_t AsTextField::TextField_get_length(FlashEditInstance* self) const
{
	return int32_t(self->getText().length());
}

int32_t AsTextField::TextField_get_maxChars(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_maxChars not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_maxChars(FlashEditInstance* self, int32_t maxChars) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_maxChars not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_maxhscroll(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_maxhscroll not implemented" << Endl;
	)
	return 0;
}

int32_t AsTextField::TextField_get_maxscroll(FlashEditInstance* self) const
{
	return self->getMaxScroll();
}

Ref< FlashContextMenu > AsTextField::TextField_get_menu(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_menu not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_menu(FlashEditInstance* self, FlashContextMenu* contextMenu) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_menu not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_mouseWheelEnabled(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_mouseWheelEnabled not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_mouseWheelEnabled(FlashEditInstance* self, bool mouseWheelEnabled) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_mouseWheelEnabled not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_multiline(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_multiline not implemented" << Endl;
	)
	return true;
}

void AsTextField::TextField_set_multiline(FlashEditInstance* self, bool multiliine) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_multiline not implemented" << Endl;
	)
}

std::string AsTextField::TextField_get_name(FlashEditInstance* self) const
{
	return self->getName();
}

void AsTextField::TextField_set_name(FlashEditInstance* self, const std::string& name) const
{
	self->setName(name);
}

Ref< FlashCharacterInstance > AsTextField::TextField_get_parent(FlashEditInstance* self) const
{
	return self->getParent();
}

void AsTextField::TextField_set_parent(FlashEditInstance* self, FlashSpriteInstance* movieClip) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_parent not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_password(FlashEditInstance* self) const
{
	return self->getPassword();
}

void AsTextField::TextField_set_password(FlashEditInstance* self, bool password) const
{
	self->setPassword(password);
}

std::wstring AsTextField::TextField_get_quality(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_quality not implemented" << Endl;
	)
	return L"BEST";
}

void AsTextField::TextField_set_quality(FlashEditInstance* self, const std::wstring& quality) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_quality not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_restrict(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_restrict not implemented" << Endl;
	)
	return L"";
}

void AsTextField::TextField_set_restrict(FlashEditInstance* self, const std::wstring& restrict) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_restrict not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_rotation(FlashEditInstance* self) const
{
	return self->getRotation();
}

void AsTextField::TextField_set_rotation(FlashEditInstance* self, avm_number_t rotation) const
{
	self->setRotation(rotation);
}

int32_t AsTextField::TextField_get_scroll(FlashEditInstance* self) const
{
	return self->getScroll();
}

void AsTextField::TextField_set_scroll(FlashEditInstance* self, int32_t scroll) const
{
	self->setScroll(scroll);
}

bool AsTextField::TextField_get_selectable(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_selectable not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_selectable(FlashEditInstance* self, bool selectable) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_selectable not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_sharpness(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_sharpness not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_sharpness(FlashEditInstance* self, avm_number_t sharpness) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_sharpness not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_soundbuftime(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_soundbuftime not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_soundbuftime(FlashEditInstance* self, avm_number_t soundbuftime) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_soundbuftime not implemented" << Endl;
	)
}

Ref< FlashStyleSheet > AsTextField::TextField_get_styleSheet(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_styleSheet not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_styleSheet(FlashEditInstance* self, FlashStyleSheet* styleSheet) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_styleSheet not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_tabEnabled(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_tabEnabled not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_tabEnabled(FlashEditInstance* self, bool tabEnabled) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_tabEnabled not implemented" << Endl;
	)
}

int32_t AsTextField::TextField_get_tabIndex(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_tabIndex not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_tabIndex(FlashEditInstance* self, int32_t tabIndex) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_tabIndex not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_target(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_target not implemented" << Endl;
	)
	return L"";
}

std::wstring AsTextField::TextField_get_text(FlashEditInstance* self) const
{
	return self->getText();
}

void AsTextField::TextField_set_text(FlashEditInstance* self, const std::wstring& text) const
{
	self->parseText(text);
}

avm_number_t AsTextField::TextField_get_textColor(FlashEditInstance* self) const
{
	Color4f textColor = self->getTextColor() * Scalar(255.0f);
	uint8_t r = uint8_t(textColor.getRed());
	uint8_t g = uint8_t(textColor.getGreen());
	uint8_t b = uint8_t(textColor.getBlue());
	return avm_number_t((r << 16) | (g << 8) | b);
}

void AsTextField::TextField_set_textColor(FlashEditInstance* self, avm_number_t textColor) const
{
	uint32_t n = uint32_t(textColor);
	uint8_t r = uint8_t((n >> 16) & 255);
	uint8_t g = uint8_t((n >> 8) & 255);
	uint8_t b = uint8_t((n) & 255);
	self->setTextColor(Color4f(r, g, b, 255.0f) / Scalar(255.0f));
}

avm_number_t AsTextField::TextField_get_textWidth(FlashEditInstance* self) const
{
	return self->getTextWidth();
}

avm_number_t AsTextField::TextField_get_textHeight(FlashEditInstance* self) const
{
	return self->getTextHeight();
}

avm_number_t AsTextField::TextField_get_thickness(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_thickness not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_thickness(FlashEditInstance* self, avm_number_t thickness) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_thickness not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_type(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_type not implemented" << Endl;
	)
	return L"input";
}

void AsTextField::TextField_set_type(FlashEditInstance* self, const std::wstring& type) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_type not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_url(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_url not implemented" << Endl;
	)
	return L"";
}

std::string AsTextField::TextField_get_variable(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_variable not implemented" << Endl;
	)
	return "";
}

void AsTextField::TextField_set_variable(FlashEditInstance* self, const std::string& url) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_variable not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_visible(FlashEditInstance* self) const
{
	return self->isVisible();
}

void AsTextField::TextField_set_visible(FlashEditInstance* self, bool visible) const
{
	self->setVisible(visible);
}

avm_number_t AsTextField::TextField_get_width(FlashEditInstance* self) const
{
	return self->getWidth();
}

void AsTextField::TextField_set_width(FlashEditInstance* self, avm_number_t width) const
{
	self->setWidth(width);
}

bool AsTextField::TextField_get_wordWrap(FlashEditInstance* self) const
{
	return self->getWordWrap();
}

void AsTextField::TextField_set_wordWrap(FlashEditInstance* self, bool wordWrap) const
{
	self->setWordWrap(wordWrap);
}

avm_number_t AsTextField::TextField_get_x(FlashEditInstance* self) const
{
	return self->getX();
}

void AsTextField::TextField_set_x(FlashEditInstance* self, avm_number_t x) const
{
	self->setX(x);
}

avm_number_t AsTextField::TextField_get_xmouse(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_xmouse not implemented" << Endl;
	)
	return 0;
}

avm_number_t AsTextField::TextField_get_xscale(FlashEditInstance* self) const
{
	return self->getXScale();
}

void AsTextField::TextField_set_xscale(FlashEditInstance* self, avm_number_t xscale) const
{
	self->setXScale(xscale);
}

avm_number_t AsTextField::TextField_get_y(FlashEditInstance* self) const
{
	return self->getY();
}

void AsTextField::TextField_set_y(FlashEditInstance* self, avm_number_t y) const
{
	self->setY(y);
}

avm_number_t AsTextField::TextField_get_ymouse(FlashEditInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_ymouse not implemented" << Endl;
	)
	return 0;
}

avm_number_t AsTextField::TextField_get_yscale(FlashEditInstance* self) const
{
	return self->getYScale();
}

void AsTextField::TextField_set_yscale(FlashEditInstance* self, avm_number_t yscale) const
{
	self->setYScale(yscale);
}

	}
}
