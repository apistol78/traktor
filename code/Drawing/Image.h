#ifndef traktor_drawing_Image_H
#define traktor_drawing_Image_H

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

	namespace drawing
	{
	
class Palette;
class IImageFilter;

/*! \brief Image class.
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
	Image();

	Image(const Image& src);

	Image(const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette = 0);
	
	virtual ~Image();
	
	/*! \brief Create a clone of the image.
	 *
	 * \param includeData Clone image pixels.
	 */
	Ref< Image > clone(bool includeData = true) const;

	/*! \brief Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param x Destination left coordinate.
	 * \param y Destination top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height);
	
	/*! \brief Clear entire image. */
	void clear(const Color4f& color);

	/*! \brief Get single pixel. */
	bool getPixel(int32_t x, int32_t y, Color4f& outColor) const;

	/*! \brief Set single pixel. */
	bool setPixel(int32_t x, int32_t y, const Color4f& color);
	
	/*! \brief Get single pixel, no boundary check. */
	void getPixelUnsafe(int32_t x, int32_t y, Color4f& outColor) const;
	
	/*! \brief Set single pixel, no boundary check. */
	void setPixelUnsafe(int32_t x, int32_t y, const Color4f& color);

	/*! \brief Get span of pixels, no boundary checks. */
	void getSpanUnsafe(int32_t y, Color4f* outSpan) const;

	/*! \brief Set span of pixels, no boundary checks. */
	void setSpanUnsafe(int32_t y, const Color4f* span);
	
	/*! \brief Apply filter on entire image. */
	void apply(const IImageFilter* imageFilter);

	/*! \brief Convert format of image. */
	void convert(const PixelFormat& intoPixelFormat, Palette* intoPalette = 0);

	/*! \brief Swap content of source image. */
	void swap(Image* source);

	/*! \brief Load image from file. */
	static Ref< Image > load(const Path& fileName);

	/*! \brief Load image from stream. */
	static Ref< Image > load(IStream* stream, const std::wstring& extension);

	/*! \brief Load image from resource. */
	static Ref< Image > load(const void* resource, uint32_t size, const std::wstring& extension);

	/*! \brief Save image as file. */
	bool save(const Path& fileName);

	/*! \brief Save image into stream. */
	bool save(IStream* stream, const std::wstring& extension);

	/*! \brief Get current image format. */
	const PixelFormat& getPixelFormat() const;
	
	/*! \brief Get width of image. */
	int32_t getWidth() const;

	/*! \brief Get height of image. */
	int32_t getHeight() const;

	/*! \brief Get image palette. */
	Ref< Palette > getPalette() const;

	/*! \brief Get read-only pointer to image data. */
	const void* getData() const;

	/*! \brief Get pointer to image data. */
	void* getData();

	/*! \brief Get size of data in bytes. */
	size_t getDataSize() const;

	/*! \brief Attach image meta data. */
	void setImageInfo(ImageInfo* imageInfo);

	/*! \brief Get image meta data. */
	Ref< ImageInfo > getImageInfo() const;

	/*! \brief Copy image. */
	Image& operator = (const Image& src);
	
private:
	PixelFormat m_pixelFormat;
	int32_t m_width;
	int32_t m_height;
	int32_t m_pitch;
	Ref< Palette > m_palette;
	size_t m_size;
	uint8_t* m_data;
	Ref< ImageInfo > m_imageInfo;
};
	
	}
}

#endif	// traktor_drawing_Image_H
