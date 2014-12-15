#include "Amalgam/Engine/Classes/DrawingClasses.h"
#include "Core/Io/IStream.h"
#include "Drawing/IImageFilter.h"
#include "Drawing/Image.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Color4f drawing_Image_getPixel(drawing::Image* image, int32_t x, int32_t y)
{
	Color4f c;
	image->getPixel(x, y, c);
	return c;
}

Ref< drawing::Image > drawing_Image_loadFromFile(const std::wstring& filePath)
{
	return drawing::Image::load(Path(filePath));
}

Ref< drawing::Image > drawing_Image_loadFromStream(IStream* stream, const std::wstring& extension)
{
	return drawing::Image::load(stream, extension);
}

bool drawing_Image_save(drawing::Image* image, const std::wstring& filePath)
{
	return image->save(Path(filePath));
}

void drawing_Image_copy_1(drawing::Image* image, const drawing::Image* src, int32_t x, int32_t y, int32_t width, int32_t height)
{
	image->copy(src, x, y, width, height);
}

void drawing_Image_copy_2(drawing::Image* image, const drawing::Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height)
{
	image->copy(src, dx, dy, x, y, width, height);
}

		}

void registerDrawingClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< drawing::IImageFilter > > classIImageFilter = new script::AutoScriptClass< drawing::IImageFilter >();
	scriptManager->registerClass(classIImageFilter);

	Ref< script::AutoScriptClass< drawing::Image > > classImage = new script::AutoScriptClass< drawing::Image >();
	classImage->addConstructor();
	classImage->addMethod("clone", &drawing::Image::clone);
	classImage->addMethod("copy", &drawing_Image_copy_1);
	classImage->addMethod("copy", &drawing_Image_copy_2);
	classImage->addMethod("clear", &drawing::Image::clear);
	classImage->addMethod("getPixel", &drawing_Image_getPixel);
	classImage->addMethod("setPixel", &drawing::Image::setPixel);
	classImage->addMethod("apply", &drawing::Image::apply);
	classImage->addStaticMethod("loadFromFile", &drawing_Image_loadFromFile);
	classImage->addStaticMethod("loadFromStream", &drawing_Image_loadFromStream);
	classImage->addMethod("save", &drawing_Image_save);
	classImage->addMethod("getWidth", &drawing::Image::getWidth);
	classImage->addMethod("getHeight", &drawing::Image::getHeight);
	scriptManager->registerClass(classImage);
}

	}
}
