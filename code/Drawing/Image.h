#pragma once

#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::drawing
{

class Palette;
class IImageFilter;
class ITransferFunction;

/*! Image class.
 * \ingroup Drawing
 *
 * Support for basic drawing operations as well as loading and saving
 * to some standard formats such as PNG and JPEG.
 * As the main purpose of this class is to hold image data of various
 * internal formats. The drawing operations are very slow as each pixel
 * gets converted into the current pixel format before it's actually
 * painted onto the image.
 */
class T_DLLCLASS Image : public Object
{
	T_RTTI_CLASS;

public:
	Image() = default;

	explicit Image(const Image& src);

	explicit Image(const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette = nullptr);

	explicit Image(void* data, const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette = nullptr);

	virtual ~Image();

	/*! Create a clone of the image.
	 *
	 * \param includeData Clone image pixels.
	 */
	Ref< Image > clone(bool includeData = true) const;

	/*! Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param x Source left coordinate.
	 * \param y Source top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height);

	/*! Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param dx Destination left coordinate.
	 * \param dy Destination top coordinate.
	 * \param x Source left coordinate.
	 * \param y Source top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height);

	/*! Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param x Source left coordinate.
	 * \param y Source top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf);

	/*! Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param dx Destination left coordinate.
	 * \param dy Destination top coordinate.
	 * \param x Source left coordinate.
	 * \param y Source top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf);

	/*! Clear entire image. */
	void clear(const Color4f& color);

	/*! Clear only alpha. */
	void clearAlpha(float alpha);

	/*! Get single pixel. */
	bool getPixel(int32_t x, int32_t y, Color4f& outColor) const;

	/*! Set single pixel. */
	bool setPixel(int32_t x, int32_t y, const Color4f& color);

	/*! Alpha blend single pixel. */
	bool setPixelAlphaBlend(int32_t x, int32_t y, const Color4f& color);

	/*! Get single pixel, no boundary check. */
	void getPixelUnsafe(int32_t x, int32_t y, Color4f& outColor) const;

	/*! Set single pixel, no boundary check. */
	void setPixelUnsafe(int32_t x, int32_t y, const Color4f& color);

	/*! Alpha blend single pixel, no boundary check. */
	void setPixelAlphaBlendUnsafe(int32_t x, int32_t y, const Color4f& color);

	/*! Get span of pixels, no boundary checks. */
	void getSpanUnsafe(int32_t y, Color4f* outSpan) const;

	/*! Set span of pixels, no boundary checks. */
	void setSpanUnsafe(int32_t y, const Color4f* span);

	/*! Get vertical span of pixels, no boundary checks. */
	void getVerticalSpanUnsafe(int32_t x, Color4f* outSpan) const;

	/*! Set vertical span of pixels, no boundary checks. */
	void setVerticalSpanUnsafe(int32_t x, const Color4f* span);

	/*! Apply filter on entire image. */
	void apply(const IImageFilter* imageFilter);

	/*! Convert format of image. */
	void convert(const PixelFormat& intoPixelFormat, Palette* intoPalette = 0);

	/*! Swap content of source image. */
	void swap(Image* source);

	/*! Load image from file. */
	static Ref< Image > load(const Path& fileName);

	/*! Load image from stream. */
	static Ref< Image > load(IStream* stream, const std::wstring& extension);

	/*! Load image from resource. */
	static Ref< Image > load(const void* resource, uint32_t size, const std::wstring& extension);

	/*! Save image as file. */
	bool save(const Path& fileName) const;

	/*! Save image into stream. */
	bool save(IStream* stream, const std::wstring& extension) const;

	/*! Get current image format. */
	const PixelFormat& getPixelFormat() const { return m_pixelFormat; }

	/*! Get width of image. */
	int32_t getWidth() const { return m_width; }

	/*! Get height of image. */
	int32_t getHeight() const { return m_height; }

	/*! Get image palette. */
	Palette* getPalette() const { return m_palette; }

	/*! Get read-only pointer to image data. */
	const void* getData() const { return m_data; }

	/*! Get pointer to image data. */
	void* getData() { return m_data; }

	/*! Get size of data in bytes. */
	size_t getDataSize() const { return m_size; }

	/*! Attach image meta data. */
	void setImageInfo(const ImageInfo* imageInfo);

	/*! Get image meta data. */
	const ImageInfo* getImageInfo() const;

	/*! Copy image. */
	Image& operator = (const Image& src);

private:
	PixelFormat m_pixelFormat;
	int32_t m_width = 0;
	int32_t m_height = 0;
	int32_t m_pitch = 0;
	Ref< Palette > m_palette;
	size_t m_size = 0;
	uint8_t* m_data = nullptr;
	bool m_own = false;
	Ref< const ImageInfo > m_imageInfo;
};

}
