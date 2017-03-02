#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/BitmapData.h"
#include "Flash/Action/Common/BitmapFilter.h"
#include "Flash/Action/Common/Point.h"
#include "Flash/Action/Common/Rectangle.h"
#include "Flash/Action/Common/Classes/As_flash_display_BitmapData.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_display_BitmapData", As_flash_display_BitmapData, ActionClass)

As_flash_display_BitmapData::As_flash_display_BitmapData(ActionContext* context)
:	ActionClass(context, "flash.display.BitmapData")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->addProperty("height", createNativeFunction(context, &BitmapData::getHeight), 0);
	prototype->addProperty("rectangle", createNativeFunction(context, &BitmapData::getRectangle), 0);
	prototype->addProperty("transparent", createNativeFunction(context, &BitmapData::getTransparent), 0);
	prototype->addProperty("width", createNativeFunction(context, &BitmapData::getWidth), 0);

	prototype->setMember("applyFilter", ActionValue(createNativeFunction(context, &BitmapData::applyFilter)));
	//prototype->setMember("clone", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_clone)));
	//prototype->setMember("colorTransform", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_colorTransform)));
	//prototype->setMember("copyChannel", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_copyChannel)));
	//prototype->setMember("copyPixels", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_copyPixels)));
	//prototype->setMember("dispose", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_dispose)));
	prototype->setMember("draw", ActionValue(createNativeFunction(context, &BitmapData::draw)));
	prototype->setMember("fillRect", ActionValue(createNativeFunction(context, &BitmapData::fillRect)));
	//prototype->setMember("floodFill", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_floodFill)));
	//prototype->setMember("generateFilterRect", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_generateFilterRect)));
	//prototype->setMember("getColorBoundsRect", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_getColorBoundsRect)));
	//prototype->setMember("getPixel", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_getPixel)));
	//prototype->setMember("getPixel32", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_getPixel32)));
	//prototype->setMember("hitTest", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_hitTest)));
	//prototype->setMember("loadBitmap", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_loadBitmap)));
	//prototype->setMember("merge", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_merge)));
	//prototype->setMember("noise", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_noise)));
	//prototype->setMember("paletteMap", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_paletteMap)));
	//prototype->setMember("perlinNoise", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_perlinNoise)));
	//prototype->setMember("pixelDissolve", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_pixelDissolve)));
	//prototype->setMember("scroll", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_scroll)));
	//prototype->setMember("setPixel", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_setPixel)));
	//prototype->setMember("setPixel32", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_setPixel32)));
	//prototype->setMember("threshold", ActionValue(createNativeFunction(context, this, &As_flash_display_BitmapData::BitmapData_threshold)));

	prototype->setMember("save", ActionValue(createNativeFunction(context, &BitmapData::save)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_BitmapData::initialize(ActionObject* self)
{
}

void As_flash_display_BitmapData::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< BitmapData > bm;
	if (args.size() >= 2)
	{
		bool transparent = false;
		uint32_t fillColor = 0x00000000;

		if (args.size() >= 3)
			transparent = args[2].getBoolean();

		if (args.size() >= 4)
			fillColor = uint32_t(args[3].getInteger());

		bm = new BitmapData(
			args[0].getInteger(),
			args[1].getInteger(),
			transparent,
			fillColor
		);
	}
	self->setRelay(bm);
}

ActionValue As_flash_display_BitmapData::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
