/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedMatrix33.h"
#include "Core/Class/Boxes/BoxedPointer.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/DrawingClassFactory.h"
#include "Drawing/IImageFilter.h"
#include "Drawing/Image.h"
#include "Drawing/ITransferFunction.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Drawing/Filters/BlurFilter.h"
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Filters/ChainFilter.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/CropFilter.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/GaussianBlurFilter.h"
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/NoiseFilter.h"
#include "Drawing/Filters/NormalizeFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/PerlinNoiseFilter.h"
#include "Drawing/Filters/PremultiplyAlphaFilter.h"
#include "Drawing/Filters/QuantizeFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SeparateAlphaFilter.h"
#include "Drawing/Filters/SharpenFilter.h"
#include "Drawing/Filters/SphereMapFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Drawing/Functions/BlendFunction.h"

namespace traktor::drawing
{
	namespace
	{

Ref< Image > Image_constructor_3(const PixelFormat* pixelFormat, uint32_t width, uint32_t height)
{
	return new Image(*pixelFormat, width, height);
}

Ref< Image > Image_constructor_4(BoxedPointer* pointer, const PixelFormat* pixelFormat, uint32_t width, uint32_t height)
{
	return new Image(pointer->ptr(), *pixelFormat, width, height);
}

Ref< CropFilter > CropFilter_constructor(int32_t anchorX, int32_t anchorY, int32_t width, int32_t height)
{
	return new CropFilter((CropFilter::AnchorType)anchorX, (CropFilter::AnchorType)anchorY, width, height);
}

Ref< ScaleFilter > ScaleFilter_constructor(int32_t width, int32_t height)
{
	return new ScaleFilter(width, height, ScaleFilter::MnAverage, ScaleFilter::MgLinear);
}

Color4f Image_getPixel_1(Image* image, int32_t x, int32_t y)
{
	Color4f c;
	image->getPixel(x, y, c);
	return c;
}

bool Image_getPixel_2(Image* image, int32_t x, int32_t y, BoxedColor4f* outColor)
{
	Color4f c;
	if (image->getPixel(x, y, c))
	{
		*outColor = BoxedColor4f(c);
		return true;
	}
	else
		return false;
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

void Image_copy_3(Image* image, const Image* src, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf)
{
	image->copy(src, x, y, width, height, tf);
}

void Image_copy_4(Image* image, const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf)
{
	image->copy(src, dx, dy, x, y, width, height, tf);
}

Ref< BlendFunction > BlendFunction_constructor(int32_t sourceFactor, int32_t destinationFactor, int32_t operation)
{
	return new BlendFunction(
		(BlendFunction::Factor)sourceFactor,
		(BlendFunction::Factor)destinationFactor,
		(BlendFunction::Operation)operation
	);
}

int32_t Raster_defineLinearGradientStyle(Raster* self, const Matrix33& gradientMatrix, const AlignedVector< Any >& colors)
{
	AlignedVector< std::pair< Color4f, float > > cs;
	for (size_t i = 0; i < colors.size(); i += 2)
	{
		cs.push_back(std::make_pair(
			CastAny< Color4f >::get(colors[i]),
			CastAny< float >::get(colors[i + 1])
		));
	}
	return self->defineLinearGradientStyle(gradientMatrix, cs);
}

int32_t Raster_defineRadialGradientStyle(Raster* self, const Matrix33& gradientMatrix, const AlignedVector< Any >& colors)
{
	AlignedVector< std::pair< Color4f, float > > cs;
	for (size_t i = 0; i < colors.size(); i += 2)
	{
		cs.push_back(std::make_pair(
			CastAny< Color4f >::get(colors[i]),
			CastAny< float >::get(colors[i + 1])
		));
	}
	return self->defineRadialGradientStyle(gradientMatrix, cs);
}

void Raster_quadricTo_4(Raster* self, const Vector2& p, const Vector2& c)
{
	self->quadricTo(p, c);
}

void Raster_quadricTo_2(Raster* self, const Vector2& p)
{
	self->quadricTo(p);
}

void Raster_cubicTo_6(Raster* self, const Vector2& p1, const Vector2& p2, const Vector2& c)
{
	self->cubicTo(p1, p2, c);
}

void Raster_cubicTo_4(Raster* self, const Vector2& p2, const Vector2& c)
{
	self->cubicTo(p2, c);
}

void Raster_fill(Raster* self, int32_t style0, int32_t style1, int32_t fillRule)
{
	self->fill(style0, style1, (Raster::FillRule)fillRule);
}

void Raster_stroke(Raster* self, int32_t style, float width, int32_t cap)
{
	self->stroke(style, width, (Raster::StrokeCap)cap);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drawing.DrawingClassFactory", 0, DrawingClassFactory, IRuntimeClassFactory)

void DrawingClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classPixelFormat = new AutoRuntimeClass< PixelFormat >();
	classPixelFormat->addStaticMethod("getP4", &PixelFormat::getP4);
	classPixelFormat->addStaticMethod("getP8", &PixelFormat::getP8);
	classPixelFormat->addStaticMethod("getA8", &PixelFormat::getA8);
	classPixelFormat->addStaticMethod("getR8", &PixelFormat::getR8);
	classPixelFormat->addStaticMethod("getR16", &PixelFormat::getR16);
	classPixelFormat->addStaticMethod("getR5G5B5", &PixelFormat::getR5G5B5);
	classPixelFormat->addStaticMethod("getR5G6B5", &PixelFormat::getR5G6B5);
	classPixelFormat->addStaticMethod("getR5G5B5A1", &PixelFormat::getR5G5B5A1);
	classPixelFormat->addStaticMethod("getR4G4B4A4", &PixelFormat::getR4G4B4A4);
	classPixelFormat->addStaticMethod("getR8G8B8", &PixelFormat::getR8G8B8);
	classPixelFormat->addStaticMethod("getB8G8R8", &PixelFormat::getB8G8R8);
	classPixelFormat->addStaticMethod("getA1R5G5B5", &PixelFormat::getA1R5G5B5);
	classPixelFormat->addStaticMethod("getX8R8G8B8", &PixelFormat::getX8R8G8B8);
	classPixelFormat->addStaticMethod("getX8B8G8R8", &PixelFormat::getX8B8G8R8);
	classPixelFormat->addStaticMethod("getR8G8B8X8", &PixelFormat::getR8G8B8X8);
	classPixelFormat->addStaticMethod("getB8G8R8X8", &PixelFormat::getB8G8R8X8);
	classPixelFormat->addStaticMethod("getA8R8G8B8", &PixelFormat::getA8R8G8B8);
	classPixelFormat->addStaticMethod("getA8B8G8R8", &PixelFormat::getA8B8G8R8);
	classPixelFormat->addStaticMethod("getR8G8B8A8", &PixelFormat::getR8G8B8A8);
	classPixelFormat->addStaticMethod("getB8G8R8A8", &PixelFormat::getB8G8R8A8);
	classPixelFormat->addStaticMethod("getR16F", &PixelFormat::getR16F);
	classPixelFormat->addStaticMethod("getR32F", &PixelFormat::getR32F);
	classPixelFormat->addStaticMethod("getARGBF16", &PixelFormat::getARGBF16);
	classPixelFormat->addStaticMethod("getARGBF32", &PixelFormat::getARGBF32);
	classPixelFormat->addStaticMethod("getRGBAF16", &PixelFormat::getRGBAF16);
	classPixelFormat->addStaticMethod("getRGBAF32", &PixelFormat::getRGBAF32);
	classPixelFormat->addStaticMethod("getABGRF16", &PixelFormat::getABGRF16);
	classPixelFormat->addStaticMethod("getABGRF32", &PixelFormat::getABGRF32);
	registrar->registerClass(classPixelFormat);

	auto classIImageFilter = new AutoRuntimeClass< IImageFilter >();
	registrar->registerClass(classIImageFilter);

	auto classITransferFunction = new AutoRuntimeClass< ITransferFunction >();
	registrar->registerClass(classITransferFunction);

	auto classImage = new AutoRuntimeClass< Image >();
	classImage->addProperty("width", &Image::getWidth);
	classImage->addProperty("height", &Image::getHeight);
	classImage->addConstructor();
	classImage->addConstructor< const PixelFormat*, uint32_t, uint32_t >(&Image_constructor_3);
	classImage->addConstructor< BoxedPointer*, const PixelFormat*, uint32_t, uint32_t >(&Image_constructor_4);
	classImage->addMethod("clone", &Image::clone);
	classImage->addMethod("copy", &Image_copy_1);
	classImage->addMethod("copy", &Image_copy_2);
	classImage->addMethod("copy", &Image_copy_3);
	classImage->addMethod("copy", &Image_copy_4);
	classImage->addMethod("clear", &Image::clear);
	classImage->addMethod("clearAlpha", &Image::clearAlpha);
	classImage->addMethod("getPixel", &Image_getPixel_1);
	classImage->addMethod("getPixel", &Image_getPixel_2);
	classImage->addMethod("setPixel", &Image::setPixel);
	classImage->addMethod("apply", &Image::apply);
	classImage->addStaticMethod("loadFromFile", &Image_loadFromFile);
	classImage->addStaticMethod("loadFromStream", &Image_loadFromStream);
	classImage->addMethod("save", &Image_save);
	registrar->registerClass(classImage);

	auto classBlurFilter = new AutoRuntimeClass< BlurFilter >();
	classBlurFilter->addConstructor< int32_t, int32_t >();
	registrar->registerClass(classBlurFilter);

	auto classBrightnessContrastFilter = new AutoRuntimeClass< BrightnessContrastFilter >();
	classBrightnessContrastFilter->addConstructor< float, float >();
	registrar->registerClass(classBrightnessContrastFilter);

	auto classChainFilter = new AutoRuntimeClass< ChainFilter >();
	classChainFilter->addConstructor();
	classChainFilter->addMethod("add", &ChainFilter::add);
	registrar->registerClass(classChainFilter);

	auto classConvolutionFilter = new AutoRuntimeClass< ConvolutionFilter >();
	classConvolutionFilter->addStaticMethod("createGaussianBlur3", &ConvolutionFilter::createGaussianBlur3);
	classConvolutionFilter->addStaticMethod("createGaussianBlur5", &ConvolutionFilter::createGaussianBlur5);
	classConvolutionFilter->addStaticMethod("createEmboss", &ConvolutionFilter::createEmboss);
	registrar->registerClass(classConvolutionFilter);

	auto classCropFilter = new AutoRuntimeClass< CropFilter >();
	classCropFilter->addConstructor< int32_t, int32_t, int32_t, int32_t >(&CropFilter_constructor);
	registrar->registerClass(classCropFilter);

	auto classDilateFilter = new AutoRuntimeClass< DilateFilter >();
	classDilateFilter->addConstructor< int32_t >();
	registrar->registerClass(classDilateFilter);

	auto classGammaFilter = new AutoRuntimeClass< GammaFilter >();
	classGammaFilter->addConstructor< float, float >();
	registrar->registerClass(classGammaFilter);

	auto classGaussianBlurFilter = new AutoRuntimeClass< GaussianBlurFilter >();
	classGaussianBlurFilter->addConstructor< int32_t >();
	registrar->registerClass(classGaussianBlurFilter);

	auto classGrayscaleFilter = new AutoRuntimeClass< GrayscaleFilter >();
	classGrayscaleFilter->addConstructor();
	registrar->registerClass(classGrayscaleFilter);

	auto classMirrorFilter = new AutoRuntimeClass< MirrorFilter >();
	classMirrorFilter->addConstructor< bool, bool >();
	registrar->registerClass(classMirrorFilter);

	auto classNoiseFilter = new AutoRuntimeClass< NoiseFilter >();
	classNoiseFilter->addConstructor< float >();
	registrar->registerClass(classNoiseFilter);

	auto classNormalizeFilter = new AutoRuntimeClass< NormalizeFilter >();
	classNormalizeFilter->addConstructor();
	registrar->registerClass(classNormalizeFilter);

	auto classNormalMapFilter = new AutoRuntimeClass< NormalMapFilter >();
	classNormalMapFilter->addConstructor< float >();
	registrar->registerClass(classNormalMapFilter);

	auto classPerlinNoiseFilter = new AutoRuntimeClass< PerlinNoiseFilter >();
	classPerlinNoiseFilter->addConstructor< int, float, float >();
	registrar->registerClass(classPerlinNoiseFilter);

	auto classPremultiplyAlphaFilter = new AutoRuntimeClass< PremultiplyAlphaFilter >();
	classPremultiplyAlphaFilter->addConstructor();
	registrar->registerClass(classPremultiplyAlphaFilter);

	auto classQuantizeFilter = new AutoRuntimeClass< QuantizeFilter >();
	classQuantizeFilter->addConstructor< int >();
	registrar->registerClass(classQuantizeFilter);

	auto classScaleFilter = new AutoRuntimeClass< ScaleFilter >();
	classScaleFilter->addConstructor< int32_t, int32_t >(&ScaleFilter_constructor);
	registrar->registerClass(classScaleFilter);

	auto classSeparateAlphaFilter = new AutoRuntimeClass< SeparateAlphaFilter >();
	classSeparateAlphaFilter->addConstructor();
	registrar->registerClass(classSeparateAlphaFilter);

	auto classSharpenFilter = new AutoRuntimeClass< SharpenFilter >();
	classSharpenFilter->addConstructor< int, float >();
	registrar->registerClass(classSharpenFilter);

	auto classSphereMapFilter = new AutoRuntimeClass< SphereMapFilter >();
	classSphereMapFilter->addConstructor();
	registrar->registerClass(classSphereMapFilter);

	auto classSwizzleFilter = new AutoRuntimeClass< SwizzleFilter >();
	classSwizzleFilter->addConstructor< const std::wstring& >();
	registrar->registerClass(classSwizzleFilter);

	auto classTonemapFilter = new AutoRuntimeClass< TonemapFilter >();
	classTonemapFilter->addConstructor();
	registrar->registerClass(classTonemapFilter);

	auto classTransformFilter = new AutoRuntimeClass< TransformFilter >();
	classTransformFilter->addConstructor< const Color4f&, const Color4f& >();
	registrar->registerClass(classTransformFilter);

	auto classBlendFunction = new AutoRuntimeClass< BlendFunction >();
	classBlendFunction->addConstant("One", Any::fromInt32(BlendFunction::One));
	classBlendFunction->addConstant("Zero", Any::fromInt32(BlendFunction::Zero));
	classBlendFunction->addConstant("SourceColor", Any::fromInt32(BlendFunction::SourceColor));
	classBlendFunction->addConstant("OneMinusSourceColor", Any::fromInt32(BlendFunction::OneMinusSourceColor));
	classBlendFunction->addConstant("DestinationColor", Any::fromInt32(BlendFunction::DestinationColor));
	classBlendFunction->addConstant("OneMinusDestinationColor", Any::fromInt32(BlendFunction::OneMinusDestinationColor));
	classBlendFunction->addConstant("SourceAlpha", Any::fromInt32(BlendFunction::SourceAlpha));
	classBlendFunction->addConstant("OneMinusSourceAlpha", Any::fromInt32(BlendFunction::OneMinusSourceAlpha));
	classBlendFunction->addConstant("DestinationAlpha", Any::fromInt32(BlendFunction::DestinationAlpha));
	classBlendFunction->addConstant("OneMinusDestinationAlpha", Any::fromInt32(BlendFunction::OneMinusDestinationAlpha));
	classBlendFunction->addConstant("Add", Any::fromInt32(BlendFunction::Add));
	classBlendFunction->addConstant("Subtract", Any::fromInt32(BlendFunction::Subtract));
	classBlendFunction->addConstant("ReverseSubtract", Any::fromInt32(BlendFunction::ReverseSubtract));
	classBlendFunction->addConstant("Min", Any::fromInt32(BlendFunction::Min));
	classBlendFunction->addConstant("Max", Any::fromInt32(BlendFunction::Max));
	classBlendFunction->addConstructor< int32_t, int32_t, int32_t >(&BlendFunction_constructor);
	registrar->registerClass(classBlendFunction);

	auto classRaster = new AutoRuntimeClass< Raster >();
	classRaster->addConstant("Butt", Any::fromInt32((int32_t)Raster::StrokeCap::Butt));
	classRaster->addConstant("Square", Any::fromInt32((int32_t)Raster::StrokeCap::Square));
	classRaster->addConstant("Round", Any::fromInt32((int32_t)Raster::StrokeCap::Round));
	classRaster->addConstant("OddEven", Any::fromInt32((int32_t)Raster::FillRule::OddEven));
	classRaster->addConstant("NonZero", Any::fromInt32((int32_t)Raster::FillRule::NonZero));
	classRaster->addConstructor();
	classRaster->addConstructor< drawing::Image* >();
	classRaster->addMethod("valid", &Raster::valid);
	classRaster->addMethod("setImage", &Raster::setImage);
	classRaster->addMethod("setMask", &Raster::setMask);
	classRaster->addMethod("clearStyles", &Raster::clearStyles);
	classRaster->addMethod("defineSolidStyle", &Raster::defineSolidStyle);
	classRaster->addMethod("defineLinearGradientStyle", &Raster_defineLinearGradientStyle);
	classRaster->addMethod("defineRadialGradientStyle", &Raster_defineRadialGradientStyle);
	classRaster->addMethod("defineImageStyle", &Raster::defineImageStyle);
	classRaster->addMethod("clear", &Raster::clear);
	classRaster->addMethod< void, const Vector2& >("moveTo", &Raster::moveTo);
	classRaster->addMethod< void, float, float >("moveTo", &Raster::moveTo);
	classRaster->addMethod< void, const Vector2& >("lineTo", &Raster::lineTo);
	classRaster->addMethod< void, float, float >("lineTo", &Raster::lineTo);
	classRaster->addMethod("quadricTo", &Raster_quadricTo_4);
	classRaster->addMethod("quadricTo", &Raster_quadricTo_2);
	classRaster->addMethod("cubicTo", &Raster_cubicTo_6);
	classRaster->addMethod("cubicTo", &Raster_cubicTo_4);
	classRaster->addMethod("close", &Raster::close);
	classRaster->addMethod< void, const Vector2&, const Vector2&, float >("rect", &Raster::rect);
	classRaster->addMethod< void, float, float, float, float, float >("rect", &Raster::rect);
	classRaster->addMethod< void, const Vector2&, float >("circle", &Raster::circle);
	classRaster->addMethod< void, float, float, float >("circle", &Raster::circle);
	classRaster->addMethod("fill", &Raster_fill);
	classRaster->addMethod("stroke", &Raster_stroke);
	classRaster->addMethod("submit", &Raster::submit);
	registrar->registerClass(classRaster);
}

}
