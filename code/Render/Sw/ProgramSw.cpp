#include "Render/Sw/ProgramSw.h"
#include "Render/Sw/SimpleTextureSw.h"
#include "Render/Sw/RenderTargetSw.h"
#include "Render/Sw/Samplers.h"
#include "Render/VertexElement.h"
#include "Core/Heap/Alloc.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramSw", ProgramSw, Program)

ProgramSw::ProgramSw(
	const std::map< handle_t, int >& parameterMap,
	const std::map< handle_t, int >& samplerMap,
	Processor::image_t vertexProgram,
	Processor::image_t pixelProgram,
	const RenderStateDesc& renderState,
	uint32_t interpolatorCount
) 
:	m_parameterMap(parameterMap)
,	m_samplerMap(samplerMap)
,	m_vertexProgram(vertexProgram)
,	m_pixelProgram(pixelProgram)
,	m_renderState(renderState)
,	m_interpolatorCount(interpolatorCount)
{
	m_parameters = (Vector4*)(allocAlign(256 * sizeof(Vector4), 16));
	for (int i = 0; i < 256; ++i)
		m_parameters[i].set(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 8; ++i)
		m_samplers[i] = gc_new< NullTextureSampler >();
}

void ProgramSw::destroy()
{
	for (int i = 0; i < 8; ++i)
		m_samplers[i] = 0;
	
	freeAlign(m_parameters);
	m_parameters = 0;
}

void ProgramSw::setFloatParameter(handle_t handle, float param)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second < 256);
		m_parameters[i->second].set(param, param, param, param);
	}
}

void ProgramSw::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second + length < 256)
		for (int j = 0; j < length; ++j)
			m_parameters[i->second + j].set(param[j], param[j], param[j], param[j]);
	}
}

void ProgramSw::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second < 256);
		m_parameters[i->second] = param;
	}
}

void ProgramSw::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second + length < 256);
		for (int j = 0; j < length; ++j)
			m_parameters[i->second + j] = param[j];
	}
}

void ProgramSw::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second + 4 < 256);
		m_parameters[i->second    ] = param.axisX();
		m_parameters[i->second + 1] = param.axisY();
		m_parameters[i->second + 2] = param.axisZ();
		m_parameters[i->second + 3] = param.translation();
	}
}

void ProgramSw::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, int >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_ASSERT (i->second + length * 4 < 256);
		for (int j = 0; j < length; ++j)
		{
			m_parameters[i->second + j * 4    ] = param[j].axisX();
			m_parameters[i->second + j * 4 + 1] = param[j].axisY();
			m_parameters[i->second + j * 4 + 2] = param[j].axisZ();
			m_parameters[i->second + j * 4 + 3] = param[j].translation();
		}
	}
}

void ProgramSw::setSamplerTexture(handle_t handle, Texture* texture)
{
	std::map< handle_t, int >::const_iterator i = m_samplerMap.find(handle);
	if (i != m_samplerMap.end())
	{
		if (is_a< SimpleTextureSw >(texture))
			m_samplers[i->second] = gc_new< SimpleTextureSampler< AddressWrap, AddressWrap > >(static_cast< SimpleTextureSw* >(texture));
		else if (is_a< RenderTargetSw >(texture))
			m_samplers[i->second] = gc_new< RenderTargetSampler< AddressWrap, AddressWrap > >(static_cast< RenderTargetSw* >(texture));
	}
}

void ProgramSw::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramSw::isOpaque() const
{
	return true;
}

	}
}
