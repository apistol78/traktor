#include <cstring>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>
#include <cmft/clcontext.h>
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/ProbeProcessor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProbeProcessor", ProbeProcessor, Object)

ProbeProcessor::ProbeProcessor()
:	m_clContext(nullptr)
{
}

ProbeProcessor::~ProbeProcessor()
{
	destroy();
}

bool ProbeProcessor::create()
{
	int32_t clLoaded = cmft::clLoad();
	if (clLoaded)
	{
		m_clContext = cmft::clInit(
			CMFT_CL_VENDOR_ANY_GPU,
			CMFT_CL_DEVICE_TYPE_GPU | CMFT_CL_DEVICE_TYPE_ACCELERATOR,
			0
		);
	}
	return true;
}

void ProbeProcessor::destroy()
{
	if (m_clContext)
	{
		cmft::clUnload();
		m_clContext = nullptr;
	}
}

bool ProbeProcessor::radiance(const CubeMap* input, float solidAngle, RefArray< CubeMap >& outRadiance) const
{
	// Convert image into 32-bit float point format.
	Ref< drawing::Image > hdrImage = input->createCrossImage()->clone();
	hdrImage->convert(drawing::PixelFormat::getRGBAF32());
	hdrImage->clearAlpha(0.0f);

	// Convert into cmft image.
	cmft::Image tmp;
	tmp.m_width    = (uint16_t)hdrImage->getWidth();
	tmp.m_height   = (uint16_t)hdrImage->getHeight();
	tmp.m_dataSize = (uint32_t)hdrImage->getDataSize();
	tmp.m_format   = cmft::TextureFormat::RGBA32F;
	tmp.m_numMips  = 1;
	tmp.m_numFaces = 1;
	tmp.m_data     = hdrImage->getData();

	cmft::Image image;
	cmft::imageCopy(image, tmp);
	cmft::imageCubemapFromCross(image);
	T_FATAL_ASSERT(image.m_width == image.m_height);

	uint32_t mipCount = (uint32_t)log2(image.m_width) + 1;
	T_ASSERT(mipCount >= 1);

	// Execute conversion filter.
	cmft::imageRadianceFilter(
		image,
		0,
		(cmft::LightingModel::Enum)cmft::LightingModel::Phong, //Brdf,
		false,
		(uint8_t)mipCount,
		20.0f, //glossScale,
		1.0f, //glossBias,
		(cmft::EdgeFixup::Enum)0,
		255,
		m_clContext
	);

	uint32_t sideSize = image.m_width;
	uint32_t offsets[CUBE_FACE_NUM][MAX_MIP_NUM] = { 0 };
	cmft::imageGetMipOffsets(offsets, image);

	outRadiance.resize(mipCount);
	for (uint32_t i = 0; i < mipCount; ++i)
	{
		uint32_t mipSize = sideSize >> i;

		Ref< CubeMap > cubeMip = new CubeMap(mipSize, drawing::PixelFormat::getRGBAF32());
		for (int32_t side = 0; side < 6; ++side)
		{
			std::memcpy(
				cubeMip->getSide(side)->getData(),
				(uint8_t*)image.m_data + offsets[side][i],
				cubeMip->getSide(side)->getDataSize()
			);
		}
		outRadiance[i] = cubeMip;
	}

	// Cleanup.
	cmft::imageUnload(image);
	return true;
}

bool ProbeProcessor::irradiance(const CubeMap* input, RefArray< CubeMap >& outIrradiance) const
{
	// Convert image into 32-bit float point format.
	Ref< drawing::Image > hdrImage = input->createCrossImage()->clone();
	hdrImage->convert(drawing::PixelFormat::getRGBAF32());
	hdrImage->clearAlpha(0.0f);

	// Convert into cmft image.
	cmft::Image tmp;
	tmp.m_width    = (uint16_t)hdrImage->getWidth();
	tmp.m_height   = (uint16_t)hdrImage->getHeight();
	tmp.m_dataSize = (uint32_t)hdrImage->getDataSize();
	tmp.m_format   = cmft::TextureFormat::RGBA32F;
	tmp.m_numMips  = 1;
	tmp.m_numFaces = 1;
	tmp.m_data     = hdrImage->getData();

	cmft::Image image;
	cmft::imageCopy(image, tmp);
	cmft::imageCubemapFromCross(image);
	T_FATAL_ASSERT(image.m_width == image.m_height);

	cmft::imageIrradianceFilterSh(
		image,
		input->getSize()
	);

	uint32_t sideSize = image.m_width;
	uint32_t mipCount = image.m_numMips;

	uint32_t offsets[CUBE_FACE_NUM][MAX_MIP_NUM] = { 0 };
	cmft::imageGetMipOffsets(offsets, image);

	outIrradiance.resize(mipCount);
	for (uint32_t i = 0; i < mipCount; ++i)
	{
		uint32_t mipSize = sideSize >> i;

		Ref< CubeMap > cubeMip = new CubeMap(mipSize, drawing::PixelFormat::getRGBAF32());
		for (int32_t side = 0; side < 6; ++side)
		{
			std::memcpy(
				cubeMip->getSide(side)->getData(),
				(uint8_t*)image.m_data + offsets[side][i],
				cubeMip->getSide(side)->getDataSize()
			);
		}
		outIrradiance[i] = cubeMip;
	}

	// Cleanup.
	cmft::imageUnload(image);
	return true;
}

	}
}
