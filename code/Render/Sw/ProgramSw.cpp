/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Float.h"
#include "Core/Memory/Alloc.h"
#include "Render/VertexElement.h"
#include "Render/Sw/CubeTextureSw.h"
#include "Render/Sw/ProgramSw.h"
#include "Render/Sw/RenderTargetSw.h"
#include "Render/Sw/Samplers.h"
#include "Render/Sw/SimpleTextureSw.h"
#include "Render/Sw/VolumeTextureSw.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void T_FORCE_INLINE checkRegister(const Vector4& r)
{
	T_ASSERT (!(isNan(r.x()) || isInfinite(r.x())));
	T_ASSERT (!(isNan(r.y()) || isInfinite(r.y())));
	T_ASSERT (!(isNan(r.z()) || isInfinite(r.z())));
	T_ASSERT (!(isNan(r.w()) || isInfinite(r.w())));
}

#define CHECK(r) checkRegister(r)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramSw", ProgramSw, IProgram)

ProgramSw::ProgramSw(
	const std::map< handle_t, std::pair< int, int > >& parameterMap,
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
	m_parameters = (Vector4*)(Alloc::acquireAlign(256 * sizeof(Vector4), 16, T_FILE_LINE));
	for (int i = 0; i < 256; ++i)
		m_parameters[i].set(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 8; ++i)
		m_samplers[i] = new NullTextureSampler();
}

void ProgramSw::destroy()
{
	for (int i = 0; i < 8; ++i)
		m_samplers[i] = 0;
	
	Alloc::freeAlign(m_parameters);
	m_parameters = 0;
}

void ProgramSw::setFloatParameter(handle_t handle, float param)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (i->second.first < 256);
		m_parameters[i->second.first].set(param, param, param, param);
		CHECK(m_parameters[i->second.first]);
	}
}

void ProgramSw::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		length = std::min(length, i->second.second);
		T_FATAL_ASSERT (i->second.first + length < 256)
		for (int j = 0; j < length; ++j)
		{
			m_parameters[i->second.first + j].set(param[j], param[j], param[j], param[j]);
			CHECK(m_parameters[i->second.first + j]);
		}
	}
}

void ProgramSw::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (i->second.first < 256);
		m_parameters[i->second.first] = param;
		CHECK(param);
	}
}

void ProgramSw::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		length = std::min(length, i->second.second);
		T_FATAL_ASSERT (i->second.first + length < 256);
		for (int j = 0; j < length; ++j)
		{
			m_parameters[i->second.first + j] = param[j];
			CHECK(m_parameters[i->second.first + j]);
		}
	}
}

void ProgramSw::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (i->second.first + 4 < 256);
		m_parameters[i->second.first    ] = param.axisX();
		m_parameters[i->second.first + 1] = param.axisY();
		m_parameters[i->second.first + 2] = param.axisZ();
		m_parameters[i->second.first + 3] = param.translation();
		CHECK(m_parameters[i->second.first    ]);
		CHECK(m_parameters[i->second.first + 1]);
		CHECK(m_parameters[i->second.first + 2]);
		CHECK(m_parameters[i->second.first + 3]);
	}
}

void ProgramSw::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, std::pair< int, int > >::const_iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		length = std::min(length, i->second.second / 4);
		T_FATAL_ASSERT (i->second.first + length * 4 < 256);
		for (int j = 0; j < length; ++j)
		{
			m_parameters[i->second.first + j * 4    ] = param[j].axisX();
			m_parameters[i->second.first + j * 4 + 1] = param[j].axisY();
			m_parameters[i->second.first + j * 4 + 2] = param[j].axisZ();
			m_parameters[i->second.first + j * 4 + 3] = param[j].translation();
			CHECK(m_parameters[i->second.first + j * 4    ]);
			CHECK(m_parameters[i->second.first + j * 4 + 1]);
			CHECK(m_parameters[i->second.first + j * 4 + 2]);
			CHECK(m_parameters[i->second.first + j * 4 + 3]);
		}
	}
}

void ProgramSw::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, int >::const_iterator i = m_samplerMap.find(handle);
	if (i != m_samplerMap.end())
	{
		if (!texture)
			return;

		Ref< ITexture > resolved = texture->resolve();
		if (!resolved)
			return;

		if (is_a< SimpleTextureSw >(resolved))
			m_samplers[i->second] = static_cast< SimpleTextureSw* >(resolved.ptr())->createSampler();
		else if (is_a< CubeTextureSw >(resolved))
			m_samplers[i->second] = new CubeTextureSampler< AddressWrap, AddressWrap, AddressWrap >(static_cast< CubeTextureSw* >(resolved.ptr()));
		else if (is_a< RenderTargetSw >(resolved))
			m_samplers[i->second] = static_cast< RenderTargetSw* >(resolved.ptr())->createSampler();
		else if (is_a< VolumeTextureSw >(resolved))
			m_samplers[i->second] = new VolumeTextureSampler< AddressWrap, AddressWrap, AddressWrap >(static_cast< VolumeTextureSw* >(resolved.ptr()));
	}
}

void ProgramSw::setStencilReference(uint32_t stencilReference)
{
}

	}
}
