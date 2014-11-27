#include "Core/Log/Log.h"
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

const uint32_t c_allRefreshRates =
	CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ |
	CELL_VIDEO_OUT_REFRESH_RATE_50HZ |
	CELL_VIDEO_OUT_REFRESH_RATE_60HZ |
	CELL_VIDEO_OUT_REFRESH_RATE_30HZ;

const ResolutionDesc c_resolutionDescs[] =
{
	{ 1920, 1080, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_1080 },
	{ 1280, 720, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_720 },
	{ 720, 576, CELL_VIDEO_OUT_REFRESH_RATE_50HZ, false, CELL_VIDEO_OUT_RESOLUTION_576 },
	{ 720, 480, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_480 },
	{ 1600, 1080, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_1600x1080 },
	{ 1440, 1080, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_1440x1080 },
	{ 1280, 1080, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_1280x1080 },
	{ 960, 1080, c_allRefreshRates, false, CELL_VIDEO_OUT_RESOLUTION_960x1080 },
	{ 1280, 720, c_allRefreshRates, true, CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING },
	{ 1024, 720, c_allRefreshRates, true, CELL_VIDEO_OUT_RESOLUTION_1024x720_3D_FRAME_PACKING },
	{ 960, 720, c_allRefreshRates, true, CELL_VIDEO_OUT_RESOLUTION_960x720_3D_FRAME_PACKING },
	{ 800, 720, c_allRefreshRates, true, CELL_VIDEO_OUT_RESOLUTION_800x720_3D_FRAME_PACKING },
	{ 640, 720, c_allRefreshRates, true, CELL_VIDEO_OUT_RESOLUTION_640x720_3D_FRAME_PACKING },
	{ 0, 0, false, 0 }
};

const ResolutionDesc* findResolutionDesc(int32_t width, int32_t height, bool stereoscopic)
{
	for (const ResolutionDesc* i = c_resolutionDescs; i->id; ++i)
	{
		if (i->width == width && i->height == height && i->stereoscopic == stereoscopic)
			return i;
	}
	return 0;
}

bool getGcmSurfaceInfo(TextureFormat textureFormat, uint8_t& outGcmTextureFormat, uint8_t& outGcmSurfaceFormat, uint8_t& outByteSize)
{
	switch (textureFormat)
	{
	case TfR8:
		outGcmTextureFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_A8R8G8B8;
		outByteSize = 4;
		break;

	case TfR8G8B8A8:
		outGcmTextureFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_A8R8G8B8;
		outByteSize = 4;
		break;

	case TfR16G16B16A16F:
		outGcmTextureFormat = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_F_W16Z16Y16X16;
		outByteSize = 4 * 2;
		break;

	case TfR32G32B32A32F:
		outGcmTextureFormat = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_F_W32Z32Y32X32;
		outByteSize = 4 * 4;
		break;

	case TfR16F:
	case TfR32F:
		outGcmTextureFormat = CELL_GCM_TEXTURE_X32_FLOAT;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_F_X32;
		outByteSize = 4;
		break;

	case TfR11G11B10F:
		outGcmTextureFormat = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		outGcmSurfaceFormat = CELL_GCM_SURFACE_F_W16Z16Y16X16;
		outByteSize = 4 * 2;
		break;

	default:
		return false;
	}
	return true;
}

bool getGcmTextureInfo(TextureFormat textureFormat, uint8_t& outGcmFormat)
{
	switch (textureFormat)
	{
	case TfR8:
		outGcmFormat = CELL_GCM_TEXTURE_B8;
		break;

	case TfR8G8B8A8:
		outGcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		break;

	case TfR16G16B16A16F:
		outGcmFormat = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		break;

	case TfR32G32B32A32F:
		outGcmFormat = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		break;

	case TfR16G16F:
		outGcmFormat = CELL_GCM_TEXTURE_Y16_X16_FLOAT;
		break;

	case TfR32F:
		outGcmFormat = CELL_GCM_TEXTURE_X32_FLOAT;
		break;

	case TfR11G11B10F:
		outGcmFormat = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		break;

	case TfDXT1:
		outGcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT1;
		break;

	case TfDXT2:
	case TfDXT3:
		outGcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT23;
		break;

	case TfDXT4:
	case TfDXT5:
		outGcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT45;
		break;

	default:
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
			{
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			}
			else if (depth == 8)	// FP16
			{
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			}
			else if (depth == 4)	// RGBA or ARGB
			{
				*((uint32_t*&)dst)++ = *((uint32_t*)src + (ypos * width + xpos));
			}
			else if (depth == 3)	// RGB
			{
				*dst++ = src[(ypos * width + xpos) * depth];
				*dst++ = src[(ypos * width + xpos) * depth + 1];
				*dst++ = src[(ypos * width + xpos) * depth + 2];
			}
			else
				T_ASSERT_M (0, L"Invalid depth size");
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

std::wstring lookupGcmError(int32_t err)
{
	switch (err)
	{
	case CELL_OK:
		return L"CELL_OK";

	case CELL_GCM_ERROR_INVALID_VALUE:
		return L"CELL_GCM_ERROR_INVALID_VALUE";

	case CELL_GCM_ERROR_INVALID_ALIGNMENT:
		return L"CELL_GCM_ERROR_INVALID_ALIGNMENT";

	case CELL_GCM_ERROR_INVALID_ENUM:
		return L"CELL_GCM_ERROR_INVALID_ENUM";

	case CELL_GCM_ERROR_ADDRESS_OVERWRAP:
		return L"CELL_GCM_ERROR_ADDRESS_OVERWRAP";

	case CELL_VIDEO_OUT_ERROR_NOT_IMPLEMENTED:
		return L"CELL_VIDEO_OUT_ERROR_NOT_IMPLEMENTED";

	case CELL_VIDEO_OUT_ERROR_ILLEGAL_CONFIGURATION:
		return L"CELL_VIDEO_OUT_ERROR_ILLEGAL_CONFIGURATION";

	case CELL_VIDEO_OUT_ERROR_ILLEGAL_PARAMETER:
		return L"CELL_VIDEO_OUT_ERROR_ILLEGAL_PARAMETER";

	case CELL_VIDEO_OUT_ERROR_PARAMETER_OUT_OF_RANGE:
		return L"CELL_VIDEO_OUT_ERROR_PARAMETER_OUT_OF_RANGE";

	case CELL_VIDEO_OUT_ERROR_DEVICE_NOT_FOUND:
		return L"CELL_VIDEO_OUT_ERROR_DEVICE_NOT_FOUND";

	case CELL_VIDEO_OUT_ERROR_UNSUPPORTED_VIDEO_OUT:
		return L"CELL_VIDEO_OUT_ERROR_UNSUPPORTED_VIDEO_OUT";

	case CELL_VIDEO_OUT_ERROR_UNSUPPORTED_DISPLAY_MODE:
		return L"CELL_VIDEO_OUT_ERROR_UNSUPPORTED_DISPLAY_MODE";

	case CELL_VIDEO_OUT_ERROR_CONDITION_BUSY:
		return L"CELL_VIDEO_OUT_ERROR_CONDITION_BUSY";

	case EINVAL:
		return L"EINVAL";
	}
	return L"Unknown";
}

	}
}
