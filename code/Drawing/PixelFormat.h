#ifndef traktor_drawing_PixelFormat_H
#define traktor_drawing_PixelFormat_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{
	
class Palette;

/*! \brief Pixel format description.
 * \ingroup Drawing
 */
class T_DLLCLASS PixelFormat : public Object
{
	T_RTTI_CLASS(PixelFormat)

public:
	PixelFormat();

	PixelFormat(
		int colorBits,
		uint32_t redBits,
		uint32_t redShift,
		uint32_t greenBits,
		uint32_t greenShift,
		uint32_t blueBits,
		uint32_t blueShift,
		uint32_t alphaBits,
		uint32_t alphaShift,
		bool palettized,
		bool floatPoint
	);

	PixelFormat(
		int colorBits,
		uint32_t redMask,
		uint32_t greenMask,
		uint32_t blueMask,
		uint32_t alphaMask,
		bool palettized,
		bool floatPoint
	);

	void convert(
		const Palette* srcPalette,
		const void* srcPixels,
		const PixelFormat* dstFormat,
		const Palette* dstPalette,
		void* dstPixels,
		int pixelCount
	) const;

	const bool isPalettized() const {
		return m_palettized;
	}

	const bool isFloatPoint() const {
		return m_floatPoint;
	}

	const int getColorBits() const {
		return m_colorBits;
	}
	
	const int getByteSize() const {
		return m_byteSize;
	}
	
	const int getRedBits() const {
		return m_redBits;
	}
	
	const int getRedShift() const {
		return m_redShift;
	}

	const int getGreenBits() const {
		return m_greenBits;
	}
	
	const int getGreenShift() const {
		return m_greenShift;
	}

	const int getBlueBits() const {
		return m_blueBits;
	}
	
	const int getBlueShift() const {
		return m_blueShift;
	}

	const int getAlphaBits() const {
		return m_alphaBits;
	}
	
	const int getAlphaShift() const {
		return m_alphaShift;
	}

	/*! \group Common pixel formats. */
	//@{

	static const PixelFormat* getP4()
	{
		static const PixelFormat pfP4(4, 0, 0, 0, 0, true, false);
		return &pfP4;
	}

	static const PixelFormat* getP8()
	{
		static const PixelFormat pfP8(8, 0, 0, 0, 0, true, false);
		return &pfP8;
	}

	static const PixelFormat* getA8()
	{
		static const PixelFormat pfA8(8, 0, 0, 0, 0xff, false, false);
		return &pfA8;
	}

	static const PixelFormat* getR5G5B5()
	{
		static const PixelFormat pfR5G5B5(15, 0x7c00, 0x03e0, 0x001f, 0, false, false);
		return &pfR5G5B5;
	}

	static const PixelFormat* getR5G6B5()
	{
		static const PixelFormat pfR5G6B5(16, 0xf800, 0x07e0, 0x001f, 0, false, false);
		return &pfR5G6B5;
	}

	static const PixelFormat* getR8G8B8()
	{
		static const PixelFormat pfR8G8B8(24, 0xff0000, 0x00ff00, 0x0000ff, 0, false, false);
		return &pfR8G8B8;
	}

	static const PixelFormat* getB8G8R8()
	{
		static const PixelFormat pfB8G8R8(24, 0x0000ff, 0x00ff00, 0xff0000, 0, false, false);
		return &pfB8G8R8;
	}

	static const PixelFormat* getA1R5G5B5()
	{
		static const PixelFormat pfA1R5G5B5(16, 0x7c00, 0x03e0, 0x001f, 0x8000, false, false);
		return &pfA1R5G5B5;
	}

	static const PixelFormat* getX8R8G8B8()
	{
		static const PixelFormat pfX8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0, false, false);
		return &pfX8R8G8B8;
	}

	static const PixelFormat* getX8B8G8R8()
	{
		static const PixelFormat pfX8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0, false, false);
		return &pfX8B8G8R8;
	}

	static const PixelFormat* getR8G8B8X8()
	{
		static const PixelFormat pfR8G8B8X8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0, false, false);
		return &pfR8G8B8X8;
	}

	static const PixelFormat* getB8G8R8X8()
	{
		static const PixelFormat pfB8G8R8X8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0, false, false);
		return &pfB8G8R8X8;
	}

	static const PixelFormat* getA8R8G8B8()
	{
		static const PixelFormat pfA8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, false, false);
		return &pfA8R8G8B8;
	}

	static const PixelFormat* getA8B8G8R8()
	{
		static const PixelFormat pfA8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000, false, false);
		return &pfA8B8G8R8;
	}

	static const PixelFormat* getR8G8B8A8()
	{
		static const PixelFormat pfR8G8B8A8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff, false, false);
		return &pfR8G8B8A8;
	}

	static const PixelFormat* getB8G8R8A8()
	{
		static const PixelFormat pfB8G8R8A8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0x000000ff, false, false);
		return &pfB8G8R8A8;
	}

	static const PixelFormat* getRGBAF32()
	{
		static const PixelFormat pfRGBAF32(128, 32, 0, 32, 32, 32, 64, 32, 96, false, true);
		return &pfRGBAF32;
	}

	//@}

private:
	bool m_palettized;	/**< If pixel format is describing palettized pixels. */
	bool m_floatPoint;
	int m_colorBits;	/**< Number of bits for an entire pixel. */
	int m_byteSize;		/**< Number of bytes for an entire pixel. */ 
	int m_redBits;		/**< Red bits, only valid for non-palettized pixels. */
	int m_redShift;
	int m_greenBits;
	int m_greenShift;
	int m_blueBits;
	int m_blueShift;
	int m_alphaBits;
	int m_alphaShift;
};
	
	}
}

#endif	// traktor_drawing_PixelFormat_H
