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

void getProgramUCode(CGprogram program, LocalMemoryObject*& outUCode)
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

int buildParameterMap(
	const ProgramResourcePs3* resource,
	CGprogram program,
	std::map< handle_t, ProgramPs3::Parameter >& outParameterMap
)
{
	const std::vector< ProgramResourcePs3::Parameter >& params = resource->getParameters();

	int32_t offset = 0;
	for (std::vector< ProgramResourcePs3::Parameter >::const_iterator i = params.begin(); i != params.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->name);

		std::vector< CGparameter > parameters;

		if (i->count > 1)
		{
			for (int32_t j = 0; j < i->count; ++j)
			{
				CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(i->name + L"[" + toString(j) + L"]").c_str());
				if (parameter)
					parameters.push_back(parameter);
			}
		}
		else
		{
			CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(i->name).c_str());
			if (parameter)
				parameters.push_back(parameter);
		}

		if (!parameters.empty())
		{
			outParameterMap[handle].parameters = parameters;
			outParameterMap[handle].offset = offset;
			outParameterMap[handle].stride = i->size;
			offset += i->size * int32_t(parameters.size());
		}
	}

	return offset;
}

		}

ProgramPs3* ProgramPs3::ms_activeProgram = 0;

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramPs3", ProgramPs3, IProgram)

ProgramPs3::ProgramPs3()
:	m_vertexProgram(0)
,	m_pixelProgram(0)
,	m_vertexShaderUCode(0)
,	m_pixelShaderUCode(0)
{
	std::memset(m_pixelTextures, 0, sizeof(m_pixelTextures));
}

ProgramPs3::~ProgramPs3()
{
	destroy();
}

bool ProgramPs3::create(const ProgramResourcePs3* resource)
{
	T_ASSERT (resource);

	int size;
	int ret;

	m_resource = resource;
	m_vertexProgram = (CGprogram)sceCgcGetBinData(resource->getVertexShaderBin());
	m_pixelProgram = (CGprogram)sceCgcGetBinData(resource->getPixelShaderBin());

	getProgramUCode(m_vertexProgram, m_vertexShaderUCode);
	getProgramUCode(m_pixelProgram, m_pixelShaderUCode);

	size = buildParameterMap(resource, m_vertexProgram, m_vertexParameterMap);
	m_vertexParameters.resize(size);

	size = buildParameterMap(resource, m_pixelProgram, m_pixelParameterMap);
	m_pixelParameters.resize(size);

	const std::vector< ProgramResourcePs3::Sampler >& samplers = resource->getPixelSamplers();
	for (std::vector< ProgramResourcePs3::Sampler >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->name);
		m_pixelSamplerMap[handle].stage = i->stage;
	}

	m_renderState = resource->getRenderState();
	return true;
}

void ProgramPs3::destroy()
{
	if (ms_activeProgram == this)
		ms_activeProgram = 0;
}

void ProgramPs3::setFloatParameter(handle_t handle, float param)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
			m_vertexParameters[i->second.offset] = param;
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
			m_pixelParameters[i->second.offset] = param;
	}
}

void ProgramPs3::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
		{
			T_ASSERT (i->second.stride == 1);

			length = std::min< int >(length, i->second.parameters.size());
			if (length > 0)
			{
				std::memcpy(
					&m_vertexParameters[i->second.offset],
					param,
					length * sizeof(float)
				);
			}
		}
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
		{
			T_ASSERT (i->second.stride == 1);

			length = std::min< int >(length, i->second.parameters.size());
			if (length > 0)
			{
				std::memcpy(
					&m_pixelParameters[i->second.offset],
					param,
					length * sizeof(float)
				);
			}
		}
	}
}

void ProgramPs3::setVectorParameter(handle_t handle, const Vector4& param)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
			param.store(&m_vertexParameters[i->second.offset]);
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
			param.store(&m_pixelParameters[i->second.offset]);
	}
}

void ProgramPs3::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
		{
			T_ASSERT (i->second.stride == 4);

			length = std::min< int >(length, i->second.parameters.size() * 4);
			for (int j = 0; j < length; ++j)
				param[j].store(&m_vertexParameters[i->second.offset + j * 4]);
		}
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
		{
			T_ASSERT (i->second.stride == 4);

			length = std::min< int >(length, i->second.parameters.size() * 4);
			for (int j = 0; j < length; ++j)
				param[j].store(&m_pixelParameters[i->second.offset + j * 4]);
		}
	}
}

void ProgramPs3::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
			param.store(&m_vertexParameters[i->second.offset]);
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
			param.store(&m_pixelParameters[i->second.offset]);
	}
}

void ProgramPs3::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	{
		std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
		if (i != m_vertexParameterMap.end())
		{
			T_ASSERT (i->second.stride == 16);

			length = std::min< int >(length, i->second.parameters.size() * 16);
			for (int j = 0; j < length; ++j)
				param[j].store(&m_vertexParameters[i->second.offset + j * 16]);
		}
	}

	{
		std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
		if (i != m_pixelParameterMap.end())
		{
			T_ASSERT (i->second.stride == 16);

			length = std::min< int >(length, i->second.parameters.size() * 16);
			for (int j = 0; j < length; ++j)
				param[j].store(&m_pixelParameters[i->second.offset + j * 16]);
		}
	}
}

void ProgramPs3::setSamplerTexture(handle_t handle, ITexture* texture)
{
	std::map< handle_t, Sampler >::iterator i = m_pixelSamplerMap.find(handle);
	if (i != m_pixelSamplerMap.end())
		m_pixelTextures[i->second.stage] = texture;
}

void ProgramPs3::setStencilReference(uint32_t stencilReference)
{
}

void ProgramPs3::bind(StateCachePs3& stateCache)
{
	stateCache.setProgram(m_vertexProgram, m_vertexShaderUCode->getPointer(), m_pixelProgram, m_pixelShaderUCode->getOffset());
	stateCache.setRenderState(m_renderState);

	for (std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.begin(); i != m_vertexParameterMap.end(); ++i)
	{
		for (uint32_t j = 0; j < i->second.parameters.size(); ++j)
		{
			cellGcmSetVertexProgramParameter(
				gCellGcmCurrentContext,
				i->second.parameters[j],
				&m_vertexParameters[i->second.offset + j * i->second.stride]
			);
		}
	}

	if (!m_pixelParameterMap.empty())
	{
		for (std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.begin(); i != m_pixelParameterMap.end(); ++i)
		{
			for (uint32_t j = 0; j < i->second.parameters.size(); ++j)
			{
				cellGcmSetFragmentProgramParameter(
					gCellGcmCurrentContext,
					m_pixelProgram,
					i->second.parameters[j],
					&m_pixelParameters[i->second.offset + j * i->second.stride],
					m_pixelShaderUCode->getOffset()
				);
			}
		}

		cellGcmSetUpdateFragmentProgramParameter(
			gCellGcmCurrentContext,
			m_pixelShaderUCode->getOffset()
		);
	}

	for (int i = 0; i < 8; ++i)
	{
		if (m_pixelTextures[i])
		{
			if (is_a< SimpleTexturePs3 >(m_pixelTextures[i]))
			{
				static_cast< SimpleTexturePs3* >(m_pixelTextures[i])->bind(i, m_renderState.samplerStates[i]);
				continue;
			}
			if (is_a< RenderTargetPs3 >(m_pixelTextures[i]))
			{
				static_cast< RenderTargetPs3* >(m_pixelTextures[i])->bind(i, m_renderState.samplerStates[i]);
				continue;
			}
		}
	}

	ms_activeProgram = this;
}

	}
}
