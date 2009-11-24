#include "Render/Ps3/TypesPs3.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

bool getGcmTextureInfo(TextureFormat textureFormat, int& outByteSize, uint8_t& outGcmFormat)
{
	switch (textureFormat)
	{
	//case TfR8:
	//	break;

	case TfR8G8B8A8:
		outByteSize = 4;
		outGcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		break;

	case TfR16G16B16A16F:
		outByteSize = 8;
		outGcmFormat = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		break;

	case TfR32G32B32A32F:
		outByteSize = 16;
		outGcmFormat = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		break;

	//case TfR16F:
	//	break;

	case TfR32F:
		outByteSize = 4;
		outGcmFormat = CELL_GCM_TEXTURE_X32_FLOAT;
		break;

	//case TfDXT1:
	//case TfDXT2:
	//case TfDXT3:
	//case TfDXT4:
	//case TfDXT5:

	default:
		log::error << "Unsupported texture format" << Endl;
		return false;
	}
	return true;
}

namespace
{

	inline uint32_t cellUtilCountLeadingZero(uint32_t x)
	{
		uint32_t y;
		uint32_t n = 32;

		y = x >> 16; if (y != 0) { n = n - 16; x = y; }
		y = x >>  8; if (y != 0) { n = n -  8; x = y; }
		y = x >>  4; if (y != 0) { n = n -  4; x = y; }
		y = x >>  2; if (y != 0) { n = n -  2; x = y; }
		y = x >>  1; if (y != 0) { return n - 2; }
		return n - x;
	}

	inline uint32_t cellUtilLog2(uint32_t x)
	{
		return 31 - cellUtilCountLeadingZero(x);
	}

	inline int cellUtilIsPowerOfTwo(uint32_t x)
	{
		return ((x & -x) == x);
	}

	inline uint32_t cellUtilRoundDownToPowOf2(uint32_t x)
	{
		if (cellUtilIsPowerOfTwo(x))
			return x;
		
		int i = 0;
		while ((x >> i) != 0)
			i++;

		return (1 << (i-1));
	}

	inline uint32_t cellUtilRoundUpToPowOf2(uint32_t x)
	{
		if (cellUtilIsPowerOfTwo(x))
			return x;
		
		int i = 0;
		while ((x >> i) != 0)
			i++;

		return (1 << (i));
	}

	void convertSwizzle(
		uint8_t *&dst,
		const uint8_t *&src,
		const uint32_t width,
		const uint32_t depth,
		const uint32_t xpos,
		const uint32_t ypos,
		const uint32_t level
	)
	{
		if (level == 1)
		{
			if (depth == 16)	// FP32
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			else if (depth == 8)	// FP16
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			else if (depth == 4)	// RGBA or ARGB
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			else if (depth == 3)	// RGB
			{
				*dst++ = src[(ypos * width + xpos) * depth];
				*dst++ = src[(ypos * width + xpos) * depth + 1];
				*dst++ = src[(ypos * width + xpos) * depth + 2];
			}
			else
				T_ASSERT (0 && "invalid depth size");
			return;
		}
		else
		{
			convertSwizzle(dst, src, width, depth, xpos, ypos, level - 1);
			convertSwizzle(dst, src, width, depth, xpos + (1U << (level - 2)), ypos, level - 1);
			convertSwizzle(dst, src, width, depth, xpos, ypos + (1U << (level - 2)), level - 1);
			convertSwizzle(dst, src, width, depth, xpos + (1U << (level - 2)), ypos + (1U << (level - 2)), level - 1);
		}
	}

}

void cellUtilConvertLinearToSwizzle(
	uint8_t *dst,
	const uint8_t *src,
	const uint32_t width,
	const uint32_t height,
	const uint32_t depth
)
{
	if (width == height)
		convertSwizzle(dst, src, width, depth, 0, 0, cellUtilLog2(width) + 1);
	else if (width > height)
	{
		uint32_t baseLevel = cellUtilLog2(width) - (cellUtilLog2(width) - cellUtilLog2(height));
		for (uint32_t i = 0; i < (1UL << (cellUtilLog2(width) - cellUtilLog2(height))); i++)
				convertSwizzle(dst, src, width, depth, (1U << baseLevel) * i, 0, baseLevel + 1);
	}
	else if (width < height)
	{
		uint32_t baseLevel = cellUtilLog2(height) - (cellUtilLog2(height) - cellUtilLog2(width));
		for (uint32_t i = 0; i < (1UL << (cellUtilLog2(height) - cellUtilLog2(width))); i++)
				convertSwizzle(dst, src, width, depth, 0, (1U << baseLevel) * i, baseLevel + 1);
	}
}

	}
}
