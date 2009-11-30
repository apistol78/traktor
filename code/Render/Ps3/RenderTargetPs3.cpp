#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/Ps3/TypesPs3.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetPs3", RenderTargetPs3, ITexture)

RenderTargetPs3::RenderTargetPs3()
:	m_width(0)
,	m_height(0)
,	m_colorSurfaceFormat(0)
,	m_colorData(0)
{
	std::memset(&m_colorTexture, 0, sizeof(m_colorTexture));
}

bool RenderTargetPs3::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	int byteSize;

	switch (desc.format)
	{
	case TfR8:
		m_colorTexture.format = CELL_GCM_TEXTURE_B8;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_B8;
		byteSize = 1;
		break;

	case TfR8G8B8A8:
		m_colorTexture.format = CELL_GCM_TEXTURE_A8R8G8B8;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_A8R8G8B8;
		byteSize = 4;
		break;

	case TfR16G16B16A16F:
		m_colorTexture.format = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_W16Z16Y16X16;
		byteSize = 4 * 2;
		break;

	case TfR32G32B32A32F:
		m_colorTexture.format = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_W32Z32Y32X32;
		byteSize = 4 * 4;
		break;

	case TfR16F:
	case TfR32F:
		m_colorTexture.format = CELL_GCM_TEXTURE_X32_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_X32;
		byteSize = 4;
		break;

	default:
		log::error << L"Unsupported render target surface format" << Endl;
		return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;

	int surfaceWidth = m_width; //(m_width & ~63) + 64;
	int surfaceHeight = m_height; //(m_height & ~63) + 64;

	m_colorTexture.format |= CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
	m_colorTexture.mipmap = 1;
	m_colorTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	m_colorTexture.cubemap = CELL_GCM_FALSE;
	m_colorTexture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_A;
	m_colorTexture.width = surfaceWidth;
	m_colorTexture.height = surfaceHeight;
	m_colorTexture.depth = 1;
	m_colorTexture.location = CELL_GCM_LOCATION_LOCAL;
	m_colorTexture.pitch = cellGcmGetTiledPitchSize(surfaceWidth * byteSize);
	m_colorTexture.offset = 0;

	uint32_t textureSize = m_colorTexture.pitch * m_colorTexture.height;

	m_colorData = LocalMemoryAllocator::getInstance().allocAlign(textureSize, 4096);
	if (cellGcmAddressToOffset(m_colorData, &m_colorTexture.offset) != CELL_OK)
		return false;

	log::info <<
		L"PS3 render target created:" << Endl <<
		L" format " << getTextureFormatName(desc.format) << Endl <<
		L" size " << m_width << L"*" << m_height << Endl <<
		L" offset " << (void*)m_colorTexture.offset << Endl <<
		L" pitch " << m_colorTexture.pitch << Endl;

	return true;
}

void RenderTargetPs3::destroy()
{
}

int RenderTargetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetPs3::getHeight() const
{
	return m_height;
}

int RenderTargetPs3::getDepth() const
{
	return 1;
}

void RenderTargetPs3::bind(int stage, const SamplerState& samplerState)
{
	cellGcmSetTextureControl(
		gCellGcmCurrentContext,
		stage,
		CELL_GCM_TRUE,
		0,
		0,
		CELL_GCM_TEXTURE_MAX_ANISO_1
	);

	if (m_colorSurfaceFormat == CELL_GCM_SURFACE_B8 || m_colorSurfaceFormat == CELL_GCM_SURFACE_A8R8G8B8)
	{
		cellGcmSetTextureFilter(
			gCellGcmCurrentContext,
			stage,
			0,
			samplerState.minFilter,
			samplerState.magFilter,
			CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
		);
	}
	else
	{
		cellGcmSetTextureFilter(
			gCellGcmCurrentContext,
			stage,
			0,
			CELL_GCM_TEXTURE_NEAREST_NEAREST,
			CELL_GCM_TEXTURE_NEAREST,
			CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
		);
	}

	cellGcmSetTextureAddress(
		gCellGcmCurrentContext,
		stage,
		samplerState.wrapU,
		samplerState.wrapV,
		samplerState.wrapW,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_NEVER,
		0
	);

	cellGcmSetTexture(
		gCellGcmCurrentContext,
		stage,
		&m_colorTexture
	);
}

	}
}
