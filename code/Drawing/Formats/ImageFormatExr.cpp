#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_EXR)

#include <cstring>
#include <tinyexr.h>

#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Drawing/Formats/ImageFormatExr.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatExr", ImageFormatExr, IImageFormat)

Ref< Image > ImageFormatExr::read(IStream* stream)
{
	AlignedVector< uint8_t > memory;

	// Load entire file into memory.
	DynamicMemoryStream ms(memory, false, true);
	if (!StreamCopy(&ms, stream).execute())
		return nullptr;

	// Parse image data.
	float* rgba = nullptr;
	int width = 0;
	int height = 0;
	if (LoadEXRFromMemory(&rgba, &width, &height, memory.c_ptr(), memory.size(), nullptr) < 0)
		return nullptr;

	memory.clear();

	// Create image.
	Ref< drawing::Image > image = new drawing::Image(
		PixelFormat::getRGBAF32(),
		width,
		height
	);
	std::memcpy(image->getData(), rgba, width * height * (4 * sizeof(float)));
	std::free(rgba);
	return image;
}

bool ImageFormatExr::write(IStream* stream, const Image* image)
{
	return false;
}

}

#endif	// DRAWING_INCLUDE_EXR
