#ifndef traktor_drawing_PixelFormat_H
#define traktor_drawing_PixelFormat_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color4f;

	namespace drawing
	{
	
class Palette;

/*! \brief Pixel format description.
 * \ingroup Drawing
 */
class T_DLLCLASS PixelFormat
{
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
		const void* T_RESTRICT srcPixels,
		const PixelFormat& dstFormat,
		const Palette* dstPalette,
		void* T_RESTRICT dstPixels,
		int pixelCount
	) const;

	/*! \brief Convert pixels to FP32 format.
	 */
	void convertTo4f(
		const Palette* srcPalette,
		const void* T_RESTRICT srcPixels,
		Color4f* T_RESTRICT dstPixels,
		int pixelCount
	) const;

	/*! \brief Convert pixels from FP32 format.
	 */
	void convertFrom4f(
		const Color4f* T_RESTRICT srcPixels,
		const Palette* dstPalette,
		void* T_RESTRICT dstPixels,
		int pixelCount
	) const;

	/*! \brief Get endian swapped format.
	 */
	PixelFormat endianSwapped() const;

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

	bool operator == (const PixelFormat& pf) const;

	bool operator != (const PixelFormat& pf) const;

	/*! \group Common pixel formats. */
	//@{

	static const PixelFormat& getP4() {
		return ms_pfP4;
	}

	static const PixelFormat& getP8() {
		return ms_pfP8;
	}

	static const PixelFormat& getA8() {
		return ms_pfA8;
	}

	static const PixelFormat& getR8() {
		return ms_pfR8;
	}

	static const PixelFormat& getR16() {
		return ms_pfR16;
	}

	static const PixelFormat& getR5G5B5() {
		return ms_pfR5G5B5;
	}

	static const PixelFormat& getR5G6B5() {
		return ms_pfR5G6B5;
	}

	static const PixelFormat& getR5G5B5A1() {
		return ms_pfR5G5B5A1;
	}

	static const PixelFormat& getR4G4B4A4() {
		return ms_pfR4G4B4A4;
	}

	static const PixelFormat& getR8G8B8() {
		return ms_pfR8G8B8;
	}

	static const PixelFormat& getB8G8R8() {
		return ms_pfB8G8R8;
	}

	static const PixelFormat& getA1R5G5B5() {
		return ms_pfA1R5G5B5;
	}

	static const PixelFormat& getX8R8G8B8() {
		return ms_pfX8R8G8B8;
	}

	static const PixelFormat& getX8B8G8R8() {
		return ms_pfX8B8G8R8;
	}

	static const PixelFormat& getR8G8B8X8() {
		return ms_pfR8G8B8X8;
	}

	static const PixelFormat& getB8G8R8X8() {
		return ms_pfB8G8R8X8;
	}

	static const PixelFormat& getA8R8G8B8() {
		return ms_pfA8R8G8B8;
	}

	static const PixelFormat& getA8B8G8R8() {
		return ms_pfA8B8G8R8;
	}

	static const PixelFormat& getR8G8B8A8() {
		return ms_pfR8G8B8A8;
	}

	static const PixelFormat& getB8G8R8A8() {
		return ms_pfB8G8R8A8;
	}

	static const PixelFormat& getR16F() {
		return ms_pfR16F;
	}

	static const PixelFormat& getR32F() {
		return ms_pfR32F;
	}

	static const PixelFormat& getARGBF16() {
		return ms_pfARGBF16;
	}

	static const PixelFormat& getARGBF32() {
		return ms_pfARGBF32;
	}

	static const PixelFormat& getRGBAF16() {
		return ms_pfRGBAF16;
	}

	static const PixelFormat& getRGBAF32() {
		return ms_pfRGBAF32;
	}

	static const PixelFormat& getABGRF16() {
		return ms_pfABGRF16;
	}

	static const PixelFormat& getABGRF32() {
		return ms_pfABGRF32;
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
	uint32_t (*m_unpack)(const void* T_RESTRICT);
	void (*m_pack)(void* T_RESTRICT, uint32_t);

	static const PixelFormat ms_pfP4;
	static const PixelFormat ms_pfP8;
	static const PixelFormat ms_pfA8;
	static const PixelFormat ms_pfR8;
	static const PixelFormat ms_pfR16;
	static const PixelFormat ms_pfR5G5B5;
	static const PixelFormat ms_pfR5G6B5;
	static const PixelFormat ms_pfR5G5B5A1;
	static const PixelFormat ms_pfR4G4B4A4;
	static const PixelFormat ms_pfR8G8B8;
	static const PixelFormat ms_pfB8G8R8;
	static const PixelFormat ms_pfA1R5G5B5;
	static const PixelFormat ms_pfX8R8G8B8;
	static const PixelFormat ms_pfX8B8G8R8;
	static const PixelFormat ms_pfR8G8B8X8;
	static const PixelFormat ms_pfB8G8R8X8;
	static const PixelFormat ms_pfA8R8G8B8;
	static const PixelFormat ms_pfA8B8G8R8;
	static const PixelFormat ms_pfR8G8B8A8;
	static const PixelFormat ms_pfB8G8R8A8;
	static const PixelFormat ms_pfR16F;
	static const PixelFormat ms_pfR32F;
	static const PixelFormat ms_pfARGBF16;
	static const PixelFormat ms_pfARGBF32;
	static const PixelFormat ms_pfRGBAF16;
	static const PixelFormat ms_pfRGBAF32;
	static const PixelFormat ms_pfABGRF16;
	static const PixelFormat ms_pfABGRF32;
};
	
	}
}

#endif	// traktor_drawing_PixelFormat_H
