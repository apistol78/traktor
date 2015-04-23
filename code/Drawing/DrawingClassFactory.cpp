#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/DrawingClassFactory.h"
#include "Drawing/IImageFilter.h"
#include "Drawing/Image.h"
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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drawing.DrawingClassFactory", 0, DrawingClassFactory, IRuntimeClassFactory)

void DrawingClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IImageFilter > > classIImageFilter = new AutoRuntimeClass< IImageFilter >();
	registrar->registerClass(classIImageFilter);

	Ref< AutoRuntimeClass< Image > > classImage = new AutoRuntimeClass< Image >();
	classImage->addConstructor();
	classImage->addMethod("clone", &Image::clone);
	classImage->addMethod("copy", &Image_copy_1);
	classImage->addMethod("copy", &Image_copy_2);
	classImage->addMethod("clear", &Image::clear);
	classImage->addMethod("getPixel", &Image_getPixel);
	classImage->addMethod("setPixel", &Image::setPixel);
	classImage->addMethod("apply", &Image::apply);
	classImage->addStaticMethod("loadFromFile", &Image_loadFromFile);
	classImage->addStaticMethod("loadFromStream", &Image_loadFromStream);
	classImage->addMethod("save", &Image_save);
	classImage->addMethod("getWidth", &Image::getWidth);
	classImage->addMethod("getHeight", &Image::getHeight);
	registrar->registerClass(classImage);

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
}

	}
}
