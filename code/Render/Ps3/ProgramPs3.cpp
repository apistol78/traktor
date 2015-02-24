#include <ppu_intrinsics.h>
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
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
	int32_t count;
};

Semaphore g_ucodeCacheLock;
std::map< uint32_t, UCodeCacheEntry > g_ucodeCache;

bool acquireProgramUCode(MemoryHeap* memoryHeap, CGprogram program, MemoryHeapObject*& outUCode)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(g_ucodeCacheLock);

	if (!program)
		return false;

	uint32_t ucodeSize = 0;
	void* ucode = 0;

	cellGcmCgInitProgram(program);
	cellGcmCgGetUCode(program, &ucode, &ucodeSize);

	if (!ucode || !ucodeSize)
		return false;

	Adler32 a32; a32.begin(); a32.feed(ucode, ucodeSize); a32.end();
	uint32_t hash = a32.get();

	std::map< uint32_t, UCodeCacheEntry >::iterator i = g_ucodeCache.find(hash);
	if (i == g_ucodeCache.end())
	{
		outUCode = memoryHeap->alloc(ucodeSize, 64, false);
		if (!outUCode || !outUCode->getPointer())
			return false;

		std::memcpy(outUCode->getPointer(), ucode, ucodeSize);

		UCodeCacheEntry entry;
		entry.ucode = outUCode;
		entry.count = 1;
		g_ucodeCache.insert(std::make_pair(hash, entry));
	}
	else
	{
		outUCode = i->second.ucode;
		i->second.count++;
	}

	return true;
}

void releaseProgramUCode(MemoryHeapObject* ucode)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(g_ucodeCacheLock);
	for (std::map< uint32_t, UCodeCacheEntry >::iterator i = g_ucodeCache.begin(); i != g_ucodeCache.end(); ++i)
	{
		if (i->second.ucode == ucode)
		{
			if (--i->second.count <= 0)
			{
				g_ucodeCache.erase(i);
				ucode->free();
				break;
			}
		}
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
,	m_patchedPixelShaderUCode(0)
,	m_patchFrame(~0UL)
,	m_patchCounter(0)
,	m_dirty(0)
,	m_counter(counter)
{
	for (uint32_t i = 0; i < PatchQueues; ++i)
	{
		for (uint32_t j = 0; j < MaxPatchInQueue; ++j)
			m_patchPixelShaderUCode[i][j] = 0;
	}

	m_targetSize[0] =
	m_targetSize[1] = 0.0f;

	++m_counter;
}

ProgramPs3::~ProgramPs3()
{
	destroy();
	--m_counter;
}

bool ProgramPs3::create(MemoryHeap* memoryHeapLocal, MemoryHeap* memoryHeapMain, const ProgramResourcePs3* resource)
{
	T_ASSERT (resource);

	m_memoryHeapLocal = memoryHeapLocal;
	m_resource = resource;

	m_vertexProgram = (CGprogram)(resource->m_vertexShaderBin.getData());
	m_pixelProgram = (CGprogram)(resource->m_pixelShaderBin.getData());

	if (!acquireProgramUCode(memoryHeapMain, m_vertexProgram, m_vertexShaderUCode))
	{
		log::error << L"Unable to create program; failed to acquire vertex ucode, possibly out-of-memory!" << Endl;
		return false;
	}

	if (!acquireProgramUCode(memoryHeapLocal, m_pixelProgram, m_pixelShaderUCode))
	{
		log::error << L"Unable to create program; failed to acquire pixel ucode, possibly out-of-memory!" << Endl;
		return false;
	}

	m_vertexScalars = resource->m_vertexScalars;
	m_pixelScalars = resource->m_pixelScalars;

	m_vertexSamplers = resource->m_vertexSamplers;
	m_pixelSamplers = resource->m_pixelSamplers;

	for (std::map< std::wstring, ScalarParameter >::const_iterator i = resource->m_scalarParameterMap.begin(); i != resource->m_scalarParameterMap.end(); ++i)
	{
		handle_t parameterHandle = getParameterHandle(i->first);
		m_scalarParameterMap.insert(std::make_pair(
			parameterHandle,
			i->second
		));
#if T_ENABLE_PATCH_WARNING
		m_parameterName[parameterHandle] = i->first;
#endif
	}

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_textureParameterMap.begin(); i != resource->m_textureParameterMap.end(); ++i)
		m_textureParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));

	m_scalarParameterData.resize(resource->m_scalarParameterDataSize, 0.0f);
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

	if (!m_pixelScalars.empty() || !m_pixelTargetSizeUCodeOffsets.empty())
	{
		// Prepare a few patch fragments in order to reduce overhead of
		// single patching fragments.
		for (uint32_t i = 0; i < PatchQueues; ++i)
		{
			for (uint32_t j = 0; j < 2; ++j)
			{
				m_patchPixelShaderUCode[i][j] = m_memoryHeapLocal->alloc(m_pixelShaderUCode->getSize(), m_pixelShaderUCode->getAlignment(), false);
				if (!m_patchPixelShaderUCode[i][j] || !m_patchPixelShaderUCode[i][j]->getPointer())
				{
					log::error << L"Unable to create program; failed to allocate pixel ucode patch image!" << Endl;
					return false;
				}
				std::memcpy(
					m_patchPixelShaderUCode[i][j]->getPointer(),
					m_pixelShaderUCode->getPointer(),
					m_pixelShaderUCode->getSize()
				);
			}
		}
	}
	else
	{
		// No parameters; ie no need to patch this fragment thus we always use same fragment.
		m_patchedPixelShaderUCode = m_pixelShaderUCode;
	}

	return true;
}

void ProgramPs3::destroy()
{
	if (ms_activeProgram == this)
		ms_activeProgram = 0;

	for (uint32_t i = 0; i < PatchQueues; ++i)
	{
		for (uint32_t j = 0; j < MaxPatchInQueue; ++j)
		{
			if (m_patchPixelShaderUCode[i][j])
			{
				m_patchPixelShaderUCode[i][j]->free();
				m_patchPixelShaderUCode[i][j] = 0;
			}
		}
	}

	if (m_pixelShaderUCode)
	{
		releaseProgramUCode(m_pixelShaderUCode);
		m_pixelShaderUCode = 0;
	}

	if (m_vertexShaderUCode)
	{
		releaseProgramUCode(m_vertexShaderUCode);
		m_vertexShaderUCode = 0;
	}

	m_patchedPixelShaderUCode = 0;
}

void ProgramPs3::setFloatParameter(handle_t handle, float param)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		if (*parameterData == param)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (F)" << Endl;
#endif

	*parameterData = param;
	m_dirty |= usage;
}

void ProgramPs3::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		bool equal = true;
		for (int32_t i = 0; i < length; ++i)
		{
			if (parameterData[i] != param[i])
			{
				equal = false;
				break;
			}
		}
		if (equal)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (F+)" << Endl;
#endif

	__builtin_memcpy(parameterData, param, length * sizeof(float));
	m_dirty |= usage;
}

void ProgramPs3::setVectorParameter(handle_t handle, const Vector4& param)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		if (Vector4::loadAligned(parameterData) == param)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (V)" << Endl;
#endif

	param.storeAligned(parameterData);
	m_dirty |= usage;
}

void ProgramPs3::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		bool equal = true;
		for (int32_t i = 0; i < length; ++i)
		{
			if (Vector4::loadAligned(parameterData + i * 4) != param[i])
			{
				equal = false;
				break;
			}
		}
		if (equal)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (V+)" << Endl;
#endif

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&parameterData[j * 4]);

	m_dirty |= usage;
}

void ProgramPs3::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		if (Matrix44::loadAligned(parameterData) == param)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (M)" << Endl;
#endif

	param.storeAligned(parameterData);
	m_dirty |= usage;
}

void ProgramPs3::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, ScalarParameter >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	float* parameterData = &m_scalarParameterData[i->second.offset];
	uint8_t usage = i->second.usage;

	if (usage & DfPixel)
	{
		bool equal = true;
		for (int32_t i = 0; i < length; ++i)
		{
			if (Matrix44::loadAligned(parameterData + i * 16) != param[i])
			{
				equal = false;
				break;
			}
		}
		if (equal)
		{
			if ((usage &= ~DfPixel) == 0)
				return;
		}
	}

#if T_ENABLE_PATCH_WARNING
	if (usage & DfPixel)
		log::debug << L"Patch caused by \"" << m_parameterName[handle] << L"\" (M+)" << Endl;
#endif

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&parameterData[j * 16]);

	m_dirty |= usage;
}

void ProgramPs3::setTextureParameter(handle_t handle, ITexture* texture)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_textureParameterMap.find(handle);
	if (i == m_textureParameterMap.end() || m_textureParameterData[i->second] == texture)
		return;

	m_textureParameterData[i->second] = texture ? texture->resolve() : 0;
	m_dirty |= DfTexture;
}

void ProgramPs3::setStencilReference(uint32_t stencilReference)
{
	m_renderState.stencilRef = stencilReference;
}

void ProgramPs3::bind(StateCachePs3& stateCache, const float targetSize[], uint32_t frameCounter, uint32_t& outPatchCounter)
{
	stateCache.setRenderState(m_renderState);

	// Check if implicit target size fragment parameter has changed.
	if (!m_pixelScalars.empty() || !m_pixelTargetSizeUCodeOffsets.empty())
	{
		if (targetSize[0] != m_targetSize[0] || targetSize[1] != m_targetSize[1])
		{
			m_dirty |= DfPixel;
			m_targetSize[0] = targetSize[0];
			m_targetSize[1] = targetSize[1];
		}
	}

	// Ensure vertex constants are updated if we aren't the active program.
	if (ms_activeProgram != this)
		m_dirty |= DfVertex;

	if (m_dirty)
	{
		// Set vertex program constants.
		if (m_dirty & DfVertex)
		{
			for (std::vector< ProgramScalar >::iterator i = m_vertexScalars.begin(); i != m_vertexScalars.end(); ++i)
				stateCache.setVertexShaderConstant(i->vertexRegisterIndex, i->vertexRegisterCount, &m_scalarParameterData[i->offset]);
		}

		// Get patched pixel shader.
		if (m_dirty & DfPixel)
		{
			if (m_patchFrame != frameCounter)
			{
				m_patchCounter = 0;
				m_patchFrame = frameCounter;
			}

			if (m_patchCounter >= MaxPatchInQueue)
			{
				log::error << L"ProgramPs3::bind failed; out of patches" << Endl;
				return;
			}

			MemoryHeapObject** patchQueue = m_patchPixelShaderUCode[m_patchFrame % PatchQueues];

			if (!patchQueue[m_patchCounter])
			{
				patchQueue[m_patchCounter] = m_memoryHeapLocal->alloc(m_pixelShaderUCode->getSize(), m_pixelShaderUCode->getAlignment(), false);
				if (!patchQueue[m_patchCounter])
				{
					log::error << L"ProgramPs3::bind failed; out of memory" << Endl;
					return;
				}
				std::memcpy(
					patchQueue[m_patchCounter]->getPointer(),
					m_pixelShaderUCode->getPointer(),
					m_pixelShaderUCode->getSize()
				);
			}

			m_patchedPixelShaderUCode = patchQueue[m_patchCounter++];

			uint8_t* patchedPixelShaderUCode = (uint8_t*)m_patchedPixelShaderUCode->getPointer();
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
				writeFragmentScalar(&uc[0], m_targetSize[0]);
				writeFragmentScalar(&uc[1], m_targetSize[1]);
			}

			++outPatchCounter;
		}

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
		m_patchedPixelShaderUCode->getOffset(),
		m_dirty & DfPixel,
		m_dirty & DfTexture
	);

	m_dirty = 0;
	ms_activeProgram = this;
}

void ProgramPs3::unbind()
{
	ms_activeProgram = 0;
}

	}
}
