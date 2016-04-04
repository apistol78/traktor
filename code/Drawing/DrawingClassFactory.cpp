#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/DrawingClassFactory.h"
#include "Drawing/IImageFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Drawing/Filters/BlurFilter.h"
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Filters/ChainFilter.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/CropFilter.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/NormalizeFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/PerlinNoiseFilter.h"
#include "Drawing/Filters/PremultiplyAlphaFilter.h"
#include "Drawing/Filters/QuantizeFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SharpenFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Filters/TransformFilter.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

class BoxedPixelFormat : public Object
{
	T_RTTI_CLASS;

public:
	BoxedPixelFormat(const PixelFormat& pf) : m_pf(pf) {}

	const PixelFormat& unbox() const { return m_pf; }

	static Ref< BoxedPixelFormat > getP4() { return new BoxedPixelFormat(PixelFormat::getP4()); }

	static Ref< BoxedPixelFormat > getP8() { return new BoxedPixelFormat(PixelFormat::getP8()); }

	static Ref< BoxedPixelFormat > getA8() { return new BoxedPixelFormat(PixelFormat::getA8()); }

	static Ref< BoxedPixelFormat > getR8() { return new BoxedPixelFormat(PixelFormat::getR8()); }

	static Ref< BoxedPixelFormat > getR16() { return new BoxedPixelFormat(PixelFormat::getR16()); }

	static Ref< BoxedPixelFormat > getR5G5B5() { return new BoxedPixelFormat(PixelFormat::getR5G5B5()); }

	static Ref< BoxedPixelFormat > getR5G6B5() { return new BoxedPixelFormat(PixelFormat::getR5G6B5()); }

	static Ref< BoxedPixelFormat > getR5G5B5A1() { return new BoxedPixelFormat(PixelFormat::getR5G5B5A1()); }

	static Ref< BoxedPixelFormat > getR4G4B4A4() { return new BoxedPixelFormat(PixelFormat::getR4G4B4A4()); }

	static Ref< BoxedPixelFormat > getR8G8B8() { return new BoxedPixelFormat(PixelFormat::getR8G8B8()); }

	static Ref< BoxedPixelFormat > getB8G8R8() { return new BoxedPixelFormat(PixelFormat::getB8G8R8()); }

	static Ref< BoxedPixelFormat > getA1R5G5B5() { return new BoxedPixelFormat(PixelFormat::getA1R5G5B5()); }

	static Ref< BoxedPixelFormat > getX8R8G8B8() { return new BoxedPixelFormat(PixelFormat::getX8R8G8B8()); }

	static Ref< BoxedPixelFormat > getX8B8G8R8() { return new BoxedPixelFormat(PixelFormat::getX8B8G8R8()); }

	static Ref< BoxedPixelFormat > getR8G8B8X8() { return new BoxedPixelFormat(PixelFormat::getR8G8B8X8()); }

	static Ref< BoxedPixelFormat > getB8G8R8X8() { return new BoxedPixelFormat(PixelFormat::getB8G8R8X8()); }

	static Ref< BoxedPixelFormat > getA8R8G8B8() { return new BoxedPixelFormat(PixelFormat::getA8R8G8B8()); }

	static Ref< BoxedPixelFormat > getA8B8G8R8() { return new BoxedPixelFormat(PixelFormat::getA8B8G8R8()); }

	static Ref< BoxedPixelFormat > getR8G8B8A8() { return new BoxedPixelFormat(PixelFormat::getR8G8B8A8()); }

	static Ref< BoxedPixelFormat > getB8G8R8A8() { return new BoxedPixelFormat(PixelFormat::getB8G8R8A8()); }

	static Ref< BoxedPixelFormat > getR16F() { return new BoxedPixelFormat(PixelFormat::getR16F()); }

	static Ref< BoxedPixelFormat > getR32F() { return new BoxedPixelFormat(PixelFormat::getR32F()); }

	static Ref< BoxedPixelFormat > getARGBF16() { return new BoxedPixelFormat(PixelFormat::getARGBF16()); }

	static Ref< BoxedPixelFormat > getARGBF32() { return new BoxedPixelFormat(PixelFormat::getARGBF32()); }

	static Ref< BoxedPixelFormat > getRGBAF16() { return new BoxedPixelFormat(PixelFormat::getRGBAF16()); }

	static Ref< BoxedPixelFormat > getRGBAF32() { return new BoxedPixelFormat(PixelFormat::getRGBAF32()); }

	static Ref< BoxedPixelFormat > getABGRF16() { return new BoxedPixelFormat(PixelFormat::getABGRF16()); }

	static Ref< BoxedPixelFormat > getABGRF32() { return new BoxedPixelFormat(PixelFormat::getABGRF32()); }

private:
	const PixelFormat& m_pf;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.PixelFormat", BoxedPixelFormat, Object)

Ref< Image > Image_constructor_3(const BoxedPixelFormat* pixelFormat, uint32_t width, uint32_t height)
{
	return new Image(pixelFormat->unbox(), width, height);
}

Ref< CropFilter > CropFilter_constructor(int32_t anchorX, int32_t anchorY, int32_t width, int32_t height)
{
	return new CropFilter((CropFilter::AnchorType)anchorX, (CropFilter::AnchorType)anchorY, width, height);
}

Ref< ScaleFilter > ScaleFilter_constructor(int32_t width, int32_t height)
{
	return new ScaleFilter(width, height, ScaleFilter::MnAverage, ScaleFilter::MgLinear);
}

Color4f Image_getPixel(Image* image, int32_t x, int32_t y)
{
	Color4f c;
	image->getPixel(x, y, c);
	return c;
}

Ref< Image > Image_loadFromFile(const std::wstring& filePath)
{
	return Image::load(Path(filePath));
}

Ref< Image > Image_loadFromStream(IStream* stream, const std::wstring& extension)
{
	return Image::load(stream, extension);
}

bool Image_save(Image* image, const std::wstring& filePath)
{
	return image->save(Path(filePath));
}

void Image_copy_1(Image* image, const Image* src, int32_t x, int32_t y, int32_t width, int32_t height)
{
	image->copy(src, x, y, width, height);
}

void Image_copy_2(Image* image, const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height)
{
	image->copy(src, dx, dy, x, y, width, height);
}

void Raster_moveTo(Raster* self, float x, float y)
{
	self->moveTo(x, y);
}

void Raster_lineTo(Raster* self, float x, float y)
{
	self->lineTo(x, y);
}

void Raster_quadricTo_4(Raster* self, float x1, float y1, float x, float y)
{
	self->quadricTo(x1, y1, x, y);
}

void Raster_quadricTo_2(Raster* self, float x, float y)
{
	self->quadricTo(x, y);
}

void Raster_cubicTo_6(Raster* self, float x1, float y1, float x2, float y2, float x, float y)
{
	self->cubicTo(x1, y1, x2, y2, x, y);
}

void Raster_cubicTo_4(Raster* self, float x2, float y2, float x, float y)
{
	self->cubicTo(x2, y2, x, y);
}

void Raster_fill(Raster* self, int32_t style0, int32_t style1, int32_t fillRule)
{
	self->fill(style0, style1, (Raster::FillRuleType)fillRule);
}

void Raster_stroke(Raster* self, int32_t style, float width, int32_t cap)
{
	self->stroke(style, width, (Raster::StrokeCapType)cap);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drawing.DrawingClassFactory", 0, DrawingClassFactory, IRuntimeClassFactory)

void DrawingClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< BoxedPixelFormat > > classBoxedPixelFormat = new AutoRuntimeClass< BoxedPixelFormat >();
	classBoxedPixelFormat->addStaticMethod("getP4", &BoxedPixelFormat::getP4);
	classBoxedPixelFormat->addStaticMethod("getP8", &BoxedPixelFormat::getP8);
	classBoxedPixelFormat->addStaticMethod("getA8", &BoxedPixelFormat::getA8);
	classBoxedPixelFormat->addStaticMethod("getR8", &BoxedPixelFormat::getR8);
	classBoxedPixelFormat->addStaticMethod("getR16", &BoxedPixelFormat::getR16);
	classBoxedPixelFormat->addStaticMethod("getR5G5B5", &BoxedPixelFormat::getR5G5B5);
	classBoxedPixelFormat->addStaticMethod("getR5G6B5", &BoxedPixelFormat::getR5G6B5);
	classBoxedPixelFormat->addStaticMethod("getR5G5B5A1", &BoxedPixelFormat::getR5G5B5A1);
	classBoxedPixelFormat->addStaticMethod("getR4G4B4A4", &BoxedPixelFormat::getR4G4B4A4);
	classBoxedPixelFormat->addStaticMethod("getR8G8B8", &BoxedPixelFormat::getR8G8B8);
	classBoxedPixelFormat->addStaticMethod("getB8G8R8", &BoxedPixelFormat::getB8G8R8);
	classBoxedPixelFormat->addStaticMethod("getA1R5G5B5", &BoxedPixelFormat::getA1R5G5B5);
	classBoxedPixelFormat->addStaticMethod("getX8R8G8B8", &BoxedPixelFormat::getX8R8G8B8);
	classBoxedPixelFormat->addStaticMethod("getX8B8G8R8", &BoxedPixelFormat::getX8B8G8R8);
	classBoxedPixelFormat->addStaticMethod("getR8G8B8X8", &BoxedPixelFormat::getR8G8B8X8);
	classBoxedPixelFormat->addStaticMethod("getB8G8R8X8", &BoxedPixelFormat::getB8G8R8X8);
	classBoxedPixelFormat->addStaticMethod("getA8R8G8B8", &BoxedPixelFormat::getA8R8G8B8);
	classBoxedPixelFormat->addStaticMethod("getA8B8G8R8", &BoxedPixelFormat::getA8B8G8R8);
	classBoxedPixelFormat->addStaticMethod("getR8G8B8A8", &BoxedPixelFormat::getR8G8B8A8);
	classBoxedPixelFormat->addStaticMethod("getB8G8R8A8", &BoxedPixelFormat::getB8G8R8A8);
	classBoxedPixelFormat->addStaticMethod("getR16F", &BoxedPixelFormat::getR16F);
	classBoxedPixelFormat->addStaticMethod("getR32F", &BoxedPixelFormat::getR32F);
	classBoxedPixelFormat->addStaticMethod("getARGBF16", &BoxedPixelFormat::getARGBF16);
	classBoxedPixelFormat->addStaticMethod("getARGBF32", &BoxedPixelFormat::getARGBF32);
	classBoxedPixelFormat->addStaticMethod("getRGBAF16", &BoxedPixelFormat::getRGBAF16);
	classBoxedPixelFormat->addStaticMethod("getRGBAF32", &BoxedPixelFormat::getRGBAF32);
	classBoxedPixelFormat->addStaticMethod("getABGRF16", &BoxedPixelFormat::getABGRF16);
	classBoxedPixelFormat->addStaticMethod("getABGRF32", &BoxedPixelFormat::getABGRF32);
	registrar->registerClass(classBoxedPixelFormat);

	Ref< AutoRuntimeClass< IImageFilter > > classIImageFilter = new AutoRuntimeClass< IImageFilter >();
	registrar->registerClass(classIImageFilter);

	Ref< AutoRuntimeClass< Image > > classImage = new AutoRuntimeClass< Image >();
	classImage->addConstructor();
	classImage->addConstructor< const BoxedPixelFormat*, uint32_t, uint32_t >(&Image_constructor_3);
	classImage->addMethod("clone", &Image::clone);
	classImage->addMethod("copy", &Image_copy_1);
	classImage->addMethod("copy", &Image_copy_2);
	classImage->addMethod("clear", &Image::clear);
	classImage->addMethod("clearAlpha", &Image::clearAlpha);
	classImage->addMethod("getPixel", &Image_getPixel);
	classImage->addMethod("setPixel", &Image::setPixel);
	classImage->addMethod("apply", &Image::apply);
	classImage->addStaticMethod("loadFromFile", &Image_loadFromFile);
	classImage->addStaticMethod("loadFromStream", &Image_loadFromStream);
	classImage->addMethod("save", &Image_save);
	classImage->addMethod("getWidth", &Image::getWidth);
	classImage->addMethod("getHeight", &Image::getHeight);
	registrar->registerClass(classImage);

	Ref< AutoRuntimeClass< BlurFilter > > classBlurFilter = new AutoRuntimeClass< BlurFilter >();
	classBlurFilter->addConstructor< int32_t, int32_t >();
	registrar->registerClass(classBlurFilter);

	Ref< AutoRuntimeClass< BrightnessContrastFilter > > classBrightnessContrastFilter = new AutoRuntimeClass< BrightnessContrastFilter >();
	classBrightnessContrastFilter->addConstructor< float, float >();
	registrar->registerClass(classBrightnessContrastFilter);

	Ref< AutoRuntimeClass< ChainFilter > > classChainFilter = new AutoRuntimeClass< ChainFilter >();
	classChainFilter->addConstructor();
	classChainFilter->addMethod("add", &ChainFilter::add);
	registrar->registerClass(classChainFilter);

	Ref< AutoRuntimeClass< ConvolutionFilter > > classConvolutionFilter = new AutoRuntimeClass< ConvolutionFilter >();
	classConvolutionFilter->addStaticMethod("createGaussianBlur3", &ConvolutionFilter::createGaussianBlur3);
	classConvolutionFilter->addStaticMethod("createGaussianBlur5", &ConvolutionFilter::createGaussianBlur5);
	classConvolutionFilter->addStaticMethod("createEmboss", &ConvolutionFilter::createEmboss);
	registrar->registerClass(classConvolutionFilter);

	Ref< AutoRuntimeClass< CropFilter > > classCropFilter = new AutoRuntimeClass< CropFilter >();
	classCropFilter->addConstructor< int32_t, int32_t, int32_t, int32_t >(&CropFilter_constructor);
	registrar->registerClass(classCropFilter);

	Ref< AutoRuntimeClass< DilateFilter > > classDilateFilter = new AutoRuntimeClass< DilateFilter >();
	classDilateFilter->addConstructor< int32_t >();
	registrar->registerClass(classDilateFilter);

	Ref< AutoRuntimeClass< GammaFilter > > classGammaFilter = new AutoRuntimeClass< GammaFilter >();
	classGammaFilter->addConstructor< float >();
	//classGammaFilter->addConstructor< float, float, float, float >();
	registrar->registerClass(classGammaFilter);

	Ref< AutoRuntimeClass< GrayscaleFilter > > classGrayscaleFilter = new AutoRuntimeClass< GrayscaleFilter >();
	classGrayscaleFilter->addConstructor();
	registrar->registerClass(classGrayscaleFilter);

	Ref< AutoRuntimeClass< MirrorFilter > > classMirrorFilter = new AutoRuntimeClass< MirrorFilter >();
	classMirrorFilter->addConstructor< bool, bool >();
	registrar->registerClass(classMirrorFilter);

	Ref< AutoRuntimeClass< NormalizeFilter > > classNormalizeFilter = new AutoRuntimeClass< NormalizeFilter >();
	classNormalizeFilter->addConstructor();
	registrar->registerClass(classNormalizeFilter);

	Ref< AutoRuntimeClass< NormalMapFilter > > classNormalMapFilter = new AutoRuntimeClass< NormalMapFilter >();
	classNormalMapFilter->addConstructor< float >();
	registrar->registerClass(classNormalMapFilter);

	Ref< AutoRuntimeClass< PerlinNoiseFilter > > classPerlinNoiseFilter = new AutoRuntimeClass< PerlinNoiseFilter >();
	classPerlinNoiseFilter->addConstructor< int, float, float >();
	registrar->registerClass(classPerlinNoiseFilter);

	Ref< AutoRuntimeClass< PremultiplyAlphaFilter > > classPremultiplyAlphaFilter = new AutoRuntimeClass< PremultiplyAlphaFilter >();
	classPremultiplyAlphaFilter->addConstructor();
	registrar->registerClass(classPremultiplyAlphaFilter);

	Ref< AutoRuntimeClass< QuantizeFilter > > classQuantizeFilter = new AutoRuntimeClass< QuantizeFilter >();
	classQuantizeFilter->addConstructor< int >();
	registrar->registerClass(classQuantizeFilter);

	Ref< AutoRuntimeClass< ScaleFilter > > classScaleFilter = new AutoRuntimeClass< ScaleFilter >();
	classScaleFilter->addConstructor< int32_t, int32_t >(&ScaleFilter_constructor);
	registrar->registerClass(classScaleFilter);

	Ref< AutoRuntimeClass< SharpenFilter > > classSharpenFilter = new AutoRuntimeClass< SharpenFilter >();
	classSharpenFilter->addConstructor< int, float >();
	registrar->registerClass(classSharpenFilter);

	Ref< AutoRuntimeClass< SwizzleFilter > > classSwizzleFilter = new AutoRuntimeClass< SwizzleFilter >();
	classSwizzleFilter->addConstructor< const std::wstring& >();
	registrar->registerClass(classSwizzleFilter);

	Ref< AutoRuntimeClass< TonemapFilter > > classTonemapFilter = new AutoRuntimeClass< TonemapFilter >();
	classTonemapFilter->addConstructor();
	registrar->registerClass(classTonemapFilter);

	Ref< AutoRuntimeClass< TransformFilter > > classTransformFilter = new AutoRuntimeClass< TransformFilter >();
	classTransformFilter->addConstructor< const Color4f&, const Color4f& >();
	registrar->registerClass(classTransformFilter);

	Ref< AutoRuntimeClass< Raster > > classRaster = new AutoRuntimeClass< Raster >();
	classRaster->addConstant("ScButt", Any::fromInteger(Raster::ScButt));
	classRaster->addConstant("ScSquare", Any::fromInteger(Raster::ScSquare));
	classRaster->addConstant("ScRound", Any::fromInteger(Raster::ScRound));
	classRaster->addConstant("FrOddEven", Any::fromInteger(Raster::FrOddEven));
	classRaster->addConstant("FrNonZero", Any::fromInteger(Raster::FrNonZero));
	classRaster->addConstructor();
	classRaster->addConstructor< drawing::Image* >();
	classRaster->addMethod("valid", &Raster::valid);
	classRaster->addMethod("setImage", &Raster::setImage);
	classRaster->addMethod("setMask", &Raster::setMask);
	classRaster->addMethod("clearStyles", &Raster::clearStyles);
	classRaster->addMethod("defineSolidStyle", &Raster::defineSolidStyle);
	//classRaster->addMethod("defineLinearGradientStyle", &Raster::defineLinearGradientStyle);
	//classRaster->addMethod("defineRadialGradientStyle", &Raster::defineRadialGradientStyle);
	classRaster->addMethod("clear", &Raster::clear);
	classRaster->addMethod("moveTo", &Raster_moveTo);
	classRaster->addMethod("lineTo", &Raster_lineTo);
	classRaster->addMethod("quadricTo", &Raster_quadricTo_4);
	classRaster->addMethod("quadricTo", &Raster_quadricTo_2);
	classRaster->addMethod("cubicTo", &Raster_cubicTo_6);
	classRaster->addMethod("cubicTo", &Raster_cubicTo_4);
	classRaster->addMethod("close", &Raster::close);
	classRaster->addMethod("rect", &Raster::rect);
	classRaster->addMethod("circle", &Raster::circle);
	classRaster->addMethod("fill", &Raster_fill);
	classRaster->addMethod("stroke", &Raster_stroke);
	classRaster->addMethod("submit", &Raster::submit);
	registrar->registerClass(classRaster);
}

	}
}
