#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/StateCachePs3.h"

using namespace cell::Gcm;

namespace traktor
{
	namespace render
	{
		namespace
		{

struct UCodeCacheEntry
{
	LocalMemoryObject* ucode;
	uint32_t count;
};

std::map< uint32_t, UCodeCacheEntry > m_ucodeCache;

void getVertexProgramUCode(CGprogram program, LocalMemoryObject*& outUCode)
{
	uint32_t ucodeSize;
	void* ucode;

	cellGcmCgInitProgram(program);
	cellGcmCgGetUCode(program, &ucode, &ucodeSize);

	Adler32 a32; a32.begin(); a32.feed(ucode, ucodeSize); a32.end();
	uint32_t hash = a32.get();

	std::map< uint32_t, UCodeCacheEntry >::iterator i = m_ucodeCache.find(hash);
	if (i == m_ucodeCache.end())
	{
		outUCode = LocalMemoryManager::getInstance().alloc(ucodeSize, 64, false);
		std::memcpy(outUCode->getPointer(), ucode, ucodeSize);

		UCodeCacheEntry entry;
		entry.ucode = outUCode;
		entry.count = 1;
		m_ucodeCache.insert(std::make_pair(hash, entry));
	}
	else
	{
		outUCode = i->second.ucode;
		i->second.count++;
	}
}

void getPixelProgramUCode(CGprogram program, LocalMemoryObject*& outUCode)
{
	uint32_t ucodeSize;
	void* ucode;

	cellGcmCgInitProgram(program);
	cellGcmCgGetUCode(program, &ucode, &ucodeSize);

	outUCode = LocalMemoryManager::getInstance().alloc(ucodeSize, 64, false);
	std::memcpy(outUCode->getPointer(), ucode, ucodeSize);
}

		}

ProgramPs3* ProgramPs3::ms_activeProgram = 0;

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramPs3", ProgramPs3, IProgram)

ProgramPs3::ProgramPs3()
:	m_vertexProgram(0)
,	m_pixelProgram(0)
,	m_vertexShaderUCode(0)
,	m_pixelShaderUCode(0)
,	m_dirty(false)
{
}

ProgramPs3::~ProgramPs3()
{
	destroy();
}

bool ProgramPs3::create(const ProgramResourcePs3* resource)
{
	T_ASSERT (resource);

	m_resource = resource;

	m_vertexProgram = (CGprogram)sceCgcGetBinData(resource->m_vertexShaderBin);
	m_pixelProgram = (CGprogram)sceCgcGetBinData(resource->m_pixelShaderBin);

	getVertexProgramUCode(m_vertexProgram, m_vertexShaderUCode);
	getPixelProgramUCode(m_pixelProgram, m_pixelShaderUCode);

	m_vertexScalars = resource->m_vertexScalars;
	m_pixelScalars = resource->m_pixelScalars;
	m_vertexSamplers = resource->m_vertexSamplers;
	m_pixelSamplers = resource->m_pixelSamplers;

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_scalarParameterMap.begin(); i != resource->m_scalarParameterMap.end(); ++i)
	{
		m_scalarParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));
	}

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_textureParameterMap.begin(); i != resource->m_textureParameterMap.end(); ++i)
		m_textureParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));

	m_scalarParameterData.resize(resource->m_scalarParameterDataSize);
	m_textureParameterData.resize(resource->m_textureParameterDataSize);

	m_renderState = resource->m_renderState;
	return true;
}

void ProgramPs3::destroy()
{
	if (ms_activeProgram == this)
		ms_activeProgram = 0;
}

void ProgramPs3::setFloatParameter(handle_t handle, float param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	m_scalarParameterData[i->second] = param;
	m_dirty = true;
}

void ProgramPs3::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	std::memcpy(&m_scalarParameterData[i->second], param, length * sizeof(float));
	m_dirty = true;
}

void ProgramPs3::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.store(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramPs3::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].store(&m_scalarParameterData[i->second + j * 4]);

	m_dirty = true;
}

void ProgramPs3::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.store(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramPs3::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].store(&m_scalarParameterData[i->second + j * 16]);

	m_dirty = true;
}

void ProgramPs3::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_textureParameterMap.find(handle);
	if (i == m_textureParameterMap.end() || m_textureParameterData[i->second] == texture)
		return;

	m_textureParameterData[i->second] = texture;
	m_dirty = true;
}

void ProgramPs3::setStencilReference(uint32_t stencilReference)
{
}

void ProgramPs3::bind(StateCachePs3& stateCache)
{
	stateCache.setProgram(m_vertexProgram, m_vertexShaderUCode->getPointer(), m_pixelProgram, m_pixelShaderUCode->getOffset());
	stateCache.setRenderState(m_renderState);

	if (m_dirty || ms_activeProgram != this)
	{
		// Set vertex program constants.
		for (std::vector< ProgramScalar >::iterator i = m_vertexScalars.begin(); i != m_vertexScalars.end(); ++i)
		{
			cellGcmSetVertexProgramParameterBlock(
				gCellGcmCurrentContext,
				i->vertexRegisterIndex,
				i->vertexRegisterCount,
				&m_scalarParameterData[i->offset]
			);
		}

		// Patch fragment program with parameters.
		uint8_t* programUCode = (uint8_t*)m_pixelShaderUCode->getPointer();
		for (std::vector< ProgramScalar >::iterator i = m_pixelScalars.begin(); i != m_pixelScalars.end(); ++i)
		{
			for (std::vector< FragmentOffset >::iterator j = i->fragmentOffsets.begin(); j != i->fragmentOffsets.end(); ++j)
			{
				// Swap 16-in-32 each dword of data.
				for (uint32_t k = 0; k < 4; ++k)
				{
					uint32_t v = *(uint32_t*)&m_scalarParameterData[i->offset + j->parameterOffset + k];
					*(uint32_t*)&programUCode[j->ucodeOffset + k * 4] = (v >> 16) | (v << 16);
				}
			}
		}

		cellGcmSetUpdateFragmentProgramParameter(
			gCellGcmCurrentContext,
			m_pixelShaderUCode->getOffset()
		);

		for (std::vector< ProgramSampler >::iterator i = m_pixelSamplers.begin(); i != m_pixelSamplers.end(); ++i)
		{
			ITexture* texture = m_textureParameterData[i->texture];
			if (texture)
			{
				if (is_a< SimpleTexturePs3 >(texture))
				{
					static_cast< SimpleTexturePs3* >(texture)->bind(i->stage, m_renderState.samplerStates[i->stage]);
					continue;
				}
				if (is_a< RenderTargetPs3 >(texture))
				{
					static_cast< RenderTargetPs3* >(texture)->bind(i->stage, m_renderState.samplerStates[i->stage]);
					continue;
				}
			}
		}
	}

	ms_activeProgram = this;
}

	}
}
