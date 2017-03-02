#include "Core/Misc/String.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsTextFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextFormat", AsTextFormat, ActionClass)

AsTextFormat::AsTextFormat(ActionContext* context)
:	ActionClass(context, "TextFormat")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsTextFormat::initialize(ActionObject* self)
{
	ActionContext* context = getContext();

	self->addProperty("align", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_align), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_align));
	self->addProperty("blockIndent", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_blockIndent), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_blockIndent));
	self->addProperty("bold", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_bold), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_bold));
	self->addProperty("bullet", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_bullet), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_bullet));
	self->addProperty("color", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_color), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_color));
	self->addProperty("font", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_font), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_font));
	self->addProperty("indent", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_indent), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_indent));
	self->addProperty("italic", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_italic), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_italic));
	self->addProperty("kerning", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_kerning), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_kerning));
	self->addProperty("leading", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_leading), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_leading));
	self->addProperty("leftMargin", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_leftMargin), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_leftMargin));
	self->addProperty("letterSpacing", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_letterSpacing), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_letterSpacing));
	self->addProperty("rightMargin", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_rightMargin), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_rightMargin));
	self->addProperty("size", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_size), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_size));
	self->addProperty("tabStops", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_tabStops), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_tabStops));
	self->addProperty("target", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_target), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_target));
	self->addProperty("underline", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_underline), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_underline));
	self->addProperty("url", createNativeFunction(context, this, &AsTextFormat::TextFormat_get_url), createNativeFunction(context, this, &AsTextFormat::TextFormat_set_url));
}

void AsTextFormat::construct(ActionObject* self, const ActionValueArray& args)
{
	self->setRelay(new FlashTextFormat(0.0f, StaLeft, 0.0f));
}

ActionValue AsTextFormat::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

std::string AsTextFormat::TextFormat_get_align(FlashTextFormat* self) const
{
	switch (self->getAlign())
	{
	case StaLeft:
		return "left";
	case StaRight:
		return "right";
	case StaCenter:
		return "center";
	case StaJustify:
		return "justify";
	default:
		return "";
	}
}

void AsTextFormat::TextFormat_set_align(FlashTextFormat* self, const std::string& value) const
{
	if (compareIgnoreCase< std::string >(value, "left") == 0)
		self->setAlign(StaLeft);
	else if (compareIgnoreCase< std::string >(value, "right") == 0)
		self->setAlign(StaRight);
	else if (compareIgnoreCase< std::string >(value, "center") == 0)
		self->setAlign(StaCenter);
	else if (compareIgnoreCase< std::string >(value, "justify") == 0)
		self->setAlign(StaJustify);
}

float AsTextFormat::TextFormat_get_blockIndent(FlashTextFormat* self) const
{
	return 0.0f;
}

void AsTextFormat::TextFormat_set_blockIndent(FlashTextFormat* self, float value) const
{
}

bool AsTextFormat::TextFormat_get_bold(FlashTextFormat* self) const
{
	return false;
}

void AsTextFormat::TextFormat_set_bold(FlashTextFormat* self, bool value) const
{
}

bool AsTextFormat::TextFormat_get_bullet(FlashTextFormat* self) const
{
	return false;
}

void AsTextFormat::TextFormat_set_bullet(FlashTextFormat* self, bool value) const
{
}

ActionValue AsTextFormat::TextFormat_get_color(FlashTextFormat* self) const
{
	return ActionValue();
}

void AsTextFormat::TextFormat_set_color(FlashTextFormat* self, const ActionValue& value) const
{
}

std::string AsTextFormat::TextFormat_get_font(FlashTextFormat* self) const
{
	return "";
}

void AsTextFormat::TextFormat_set_font(FlashTextFormat* self, const std::string& value) const
{
}

float AsTextFormat::TextFormat_get_indent(FlashTextFormat* self) const
{
	return 0.0f;
}

void AsTextFormat::TextFormat_set_indent(FlashTextFormat* self, float value) const
{
}

bool AsTextFormat::TextFormat_get_italic(FlashTextFormat* self) const
{
	return false;
}

void AsTextFormat::TextFormat_set_italic(FlashTextFormat* self, bool value) const
{
}

bool AsTextFormat::TextFormat_get_kerning(FlashTextFormat* self) const
{
	return true;
}

void AsTextFormat::TextFormat_set_kerning(FlashTextFormat* self, bool value) const
{
}

float AsTextFormat::TextFormat_get_leading(FlashTextFormat* self) const
{
	return 0.0f;
}

void AsTextFormat::TextFormat_set_leading(FlashTextFormat* self, float value) const
{
}

float AsTextFormat::TextFormat_get_leftMargin(FlashTextFormat* self) const
{
	return 0.0f;
}

void AsTextFormat::TextFormat_set_leftMargin(FlashTextFormat* self, float value) const
{
}

float AsTextFormat::TextFormat_get_letterSpacing(FlashTextFormat* self) const
{
	return self->getLetterSpacing();
}

void AsTextFormat::TextFormat_set_letterSpacing(FlashTextFormat* self, float value) const
{
	self->setLetterSpacing(value);
}

float AsTextFormat::TextFormat_get_rightMargin(FlashTextFormat* self) const
{
	return 0.0f;
}

void AsTextFormat::TextFormat_set_rightMargin(FlashTextFormat* self, float value) const
{
}

float AsTextFormat::TextFormat_get_size(FlashTextFormat* self) const
{
	return self->getSize();
}

void AsTextFormat::TextFormat_set_size(FlashTextFormat* self, float value) const
{
	self->setSize(value);
}

ActionValue AsTextFormat::TextFormat_get_tabStops(FlashTextFormat* self) const
{
	return ActionValue();
}

void AsTextFormat::TextFormat_set_tabStops(FlashTextFormat* self, const ActionValue& value) const
{
}

std::string AsTextFormat::TextFormat_get_target(FlashTextFormat* self) const
{
	return "";
}

void AsTextFormat::TextFormat_set_target(FlashTextFormat* self, const std::string& value) const
{
}

bool AsTextFormat::TextFormat_get_underline(FlashTextFormat* self) const
{
	return false;
}

void AsTextFormat::TextFormat_set_underline(FlashTextFormat* self, bool value) const
{
}

std::string AsTextFormat::TextFormat_get_url(FlashTextFormat* self) const
{
	return "";
}

void AsTextFormat::TextFormat_set_url(FlashTextFormat* self, const std::string& value) const
{
}

	}
}
