#include <ppu_intrinsics.h>
#include "Core/Log/Log.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
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
	MemoryHeapObject* ucode;
	uint32_t count;
};

std::map< uint32_t, UCodeCacheEntry > m_ucodeCache;

void getProgramUCode(MemoryHeap* memoryHeap, CGprogram program, MemoryHeapObject*& outUCode)
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
		outUCode = memoryHeap->alloc(ucodeSize, 64, false);
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

void writeFragmentScalar(uint32_t* uc, float scalar)
{
	uint32_t sv = *(const uint32_t*)&scalar;
	*uc = (sv >> 16) | (sv << 16);
}

		}

ProgramPs3* ProgramPs3::ms_activeProgram = 0;

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramPs3", ProgramPs3, IProgram)

ProgramPs3::ProgramPs3(int32_t& counter)
:	m_vertexProgram(0)
,	m_pixelProgram(0)
,	m_vertexShaderUCode(0)
,	m_pixelShaderUCode(0)
,	m_patchedPixelShaderOffset(0)
,	m_dirty(false)
,	m_counter(counter)
{
	++m_counter;
}

ProgramPs3::~ProgramPs3()
{
	destroy();
	--m_counter;
}

bool ProgramPs3::create(MemoryHeap* memoryHeap, const ProgramResourcePs3* resource)
{
	T_ASSERT (resource);

	m_resource = resource;

	m_vertexProgram = (CGprogram)sceCgcGetBinData(resource->m_vertexShaderBin);
	m_pixelProgram = (CGprogram)sceCgcGetBinData(resource->m_pixelShaderBin);

	getProgramUCode(memoryHeap, m_vertexProgram, m_vertexShaderUCode);
	getProgramUCode(memoryHeap, m_pixelProgram, m_pixelShaderUCode);

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

	// Get patch offsets of internal parameters.
	CGparameter parameter = cellGcmCgGetNamedParameter(m_pixelProgram, "_cg_targetSize");
	if (parameter)
	{
		uint32_t constantCount = cellGcmCgGetEmbeddedConstantCount(m_pixelProgram, parameter);
		for (uint32_t k = 0; k < constantCount; ++k)
		{
			uint32_t constantOffset = cellGcmCgGetEmbeddedConstantOffset(m_pixelProgram, parameter, k);
			m_pixelTargetSizeUCodeOffsets.push_back(constantOffset);
		}
	}

	m_inputSignature = resource->m_inputSignature;
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

	__builtin_memcpy(&m_scalarParameterData[i->second], param, length * sizeof(float));
	m_dirty = true;
}

void ProgramPs3::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeAligned(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramPs3::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&m_scalarParameterData[i->second + j * 4]);

	m_dirty = true;
}

void ProgramPs3::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeAligned(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramPs3::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&m_scalarParameterData[i->second + j * 16]);

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
	m_renderState.stencilRef = stencilReference;
}

void ProgramPs3::bind(PoolAllocator& patchProgramPool, StateCachePs3& stateCache, float targetSize[])
{
	stateCache.setRenderState(m_renderState);

	if (m_dirty || ms_activeProgram != this)
	{
		// Set vertex program constants.
		for (std::vector< ProgramScalar >::iterator i = m_vertexScalars.begin(); i != m_vertexScalars.end(); ++i)
			stateCache.setVertexShaderConstant(i->vertexRegisterIndex, i->vertexRegisterCount, &m_scalarParameterData[i->offset]);

		stateCache.setVertexShaderConstant(0, 1, targetSize);

		// Get patched pixel shader.
		if (!m_pixelScalars.empty() || !m_pixelTargetSizeUCodeOffsets.empty())
		{
			uint32_t ucodeSize = alignUp(m_pixelShaderUCode->getSize(), 64);

			uint8_t* patchedPixelShaderUCode = static_cast< uint8_t* >(patchProgramPool.alloc(ucodeSize));
			T_ASSERT_M (patchedPixelShaderUCode, L"Out of memory for patched pixel programs");

			__builtin_memcpy(
				patchedPixelShaderUCode,
				m_pixelShaderUCode->getPointer(),
				m_pixelShaderUCode->getSize()
			);

			for (std::vector< ProgramScalar >::const_iterator i = m_pixelScalars.begin(); i != m_pixelScalars.end(); ++i)
			{
				for (std::vector< FragmentOffset >::const_iterator j = i->fragmentOffsets.begin(); j != i->fragmentOffsets.end(); ++j)
				{
					const float* sv = &m_scalarParameterData[i->offset + j->parameterOffset];
					uint32_t* uc = (uint32_t*)&patchedPixelShaderUCode[j->ucodeOffset];

					for (uint32_t k = 0; k < 4; ++k)
						writeFragmentScalar(&uc[k], sv[k]);
				}
			}

			for (std::vector< uint32_t >::const_iterator i = m_pixelTargetSizeUCodeOffsets.begin(); i != m_pixelTargetSizeUCodeOffsets.end(); ++i)
			{
				uint32_t* uc = (uint32_t*)&patchedPixelShaderUCode[*i];
				writeFragmentScalar(&uc[0], targetSize[0]);
				writeFragmentScalar(&uc[1], targetSize[1]);
			}

			cellGcmAddressToOffset(patchedPixelShaderUCode, &m_patchedPixelShaderOffset);
		}
		else
			m_patchedPixelShaderOffset = m_pixelShaderUCode->getOffset();

		for (std::vector< ProgramSampler >::iterator i = m_pixelSamplers.begin(); i != m_pixelSamplers.end(); ++i)
		{
			ITexture* texture = m_textureParameterData[i->texture];
			if (texture)
			{
				if (SimpleTexturePs3* simpleTexture = dynamic_type_cast< SimpleTexturePs3* >(texture))
					simpleTexture->bind(stateCache, i->stage, m_renderState.samplerStates[i->stage]);
				else if (RenderTargetPs3* renderTarget = dynamic_type_cast< RenderTargetPs3* >(texture))
					renderTarget->bind(stateCache, i->stage, m_renderState.samplerStates[i->stage]);
			}
		}
	}

	stateCache.setProgram(
		m_vertexProgram,
		m_vertexShaderUCode->getPointer(),
		m_pixelProgram,
		m_patchedPixelShaderOffset,
		(m_dirty || ms_activeProgram != this)
	);

	ms_activeProgram = this;
}

void ProgramPs3::unbind()
{
	ms_activeProgram = 0;
}

	}
}
