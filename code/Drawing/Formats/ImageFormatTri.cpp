#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Formats/ImageFormatTri.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatTri", ImageFormatTri, IImageFormat)

Ref< Image > ImageFormatTri::read(IStream* stream)
{
	Reader reader(stream);

	uint32_t magic;
	reader >> magic;
	if (magic != 'TRIF')
		return nullptr;

	uint8_t version;
	reader >> version;
	if (version != 1)
		return nullptr;

	int32_t width, height;
	reader >> width;
	reader >> height;

	bool palettized;
	reader >> palettized;

	bool floatPoint;
	reader >> floatPoint;

	bool info;
	reader >> info;

	int32_t colorBits;
	reader >> colorBits;

	int32_t redBits, redShift;
	reader >> redBits;
	reader >> redShift;

	int32_t greenBits, greenShift;
	reader >> greenBits;
	reader >> greenShift;

	int32_t blueBits, blueShift;
	reader >> blueBits;
	reader >> blueShift;

	int32_t alphaBits, alphaShift;
	reader >> alphaBits;
	reader >> alphaShift;

	PixelFormat pf(
		colorBits,
		redBits,
		redShift,
		greenBits,
		greenShift,
		blueBits,
		blueShift,
		alphaBits,
		alphaShift,
		palettized,
		floatPoint
	);

	Ref< Image > image = new Image(pf, width, height);

	int64_t dataSize = image->getDataSize();
	if (reader.read(image->getData(), dataSize) != dataSize)
		return nullptr;

	return image;
}

bool ImageFormatTri::write(IStream* stream, const Image* image)
{
	const PixelFormat& pf = image->getPixelFormat();

	Writer writer(stream);
	writer << uint32_t('TRIF');
	writer << uint8_t(1);
	writer << image->getWidth();
	writer << image->getHeight();
	writer << bool(pf.isPalettized());
	writer << bool(pf.isFloatPoint());
	writer << bool(false);
	writer << pf.getColorBits();
	writer << pf.getRedBits();
	writer << pf.getRedShift();
	writer << pf.getGreenBits();
	writer << pf.getGreenShift();
	writer << pf.getBlueBits();
	writer << pf.getBlueShift();
	writer << pf.getAlphaBits();
	writer << pf.getAlphaShift();

	int64_t dataSize = image->getDataSize();
	if (writer.write(image->getData(), dataSize) != dataSize)
		return false;

	return true;
}

}
