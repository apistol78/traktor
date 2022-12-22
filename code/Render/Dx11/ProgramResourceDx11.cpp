/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Dx11/Blob.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberBlob : public MemberComplex
{
public:
	MemberBlob(const wchar_t* const name, Ref< Blob >& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		uint8_t blob[256 * 1024];
		uint32_t blobSize;

		if (s.getDirection() == ISerializer::Direction::Read)
		{
			blobSize = sizeof(blob);
			s >> Member< void* >(
				getName(),
				[&]() { return blobSize; },	// get blob size
				[&](size_t size) { return true; },	// set blob size
				[&]() { return blob; }
			);

			m_ref = Blob::create(blobSize);
			if (!m_ref)
			{
				s.failure();
				return;
			}

			std::memcpy(m_ref->getData(), blob, blobSize);
		}
		else	// Direction::Write
		{
			blobSize = m_ref ? m_ref->getSize() : 0;
			T_ASSERT(blobSize < sizeof(blob));

			if (m_ref)
				std::memcpy(blob, m_ref->getData(), blobSize);

			s >> Member< void* >(
				getName(),
				[&]() { return blobSize; },	// get blob size
				[&](size_t size) { return true; },	// set blob size
				[&]() { return blob; }
			);
		}
	}

private:
	Ref< Blob >& m_ref;
};

class MemberD3D11_RASTERIZER_DESC : public MemberComplex
{
public:
	MemberD3D11_RASTERIZER_DESC(const wchar_t* const name, D3D11_RASTERIZER_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		size_t size = sizeof(m_ref);
		s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_RASTERIZER_DESC& m_ref;
};

class MemberD3D11_DEPTH_STENCIL_DESC : public MemberComplex
{
public:
	MemberD3D11_DEPTH_STENCIL_DESC(const wchar_t* const name, D3D11_DEPTH_STENCIL_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		size_t size = sizeof(m_ref);
		s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_DEPTH_STENCIL_DESC& m_ref;
};

class MemberD3D11_BLEND_DESC : public MemberComplex
{
public:
	MemberD3D11_BLEND_DESC(const wchar_t* const name, D3D11_BLEND_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		size_t size = sizeof(m_ref);
		s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_BLEND_DESC& m_ref;
};

class MemberD3D11_SAMPLER_DESC : public MemberComplex
{
public:
	MemberD3D11_SAMPLER_DESC(const wchar_t* const name, D3D11_SAMPLER_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		size_t size = sizeof(m_ref);
		s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_SAMPLER_DESC& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx11", 0, ProgramResourceDx11, ProgramResource)

void ProgramResourceDx11::serialize(ISerializer& s)
{
	s >> MemberBlob(L"vertexShader", m_vertexShader);
	s >> MemberBlob(L"pixelShader", m_pixelShader);

	s >> Member< uint32_t >(L"vertexShaderHash", m_vertexShaderHash);
	s >> Member< uint32_t >(L"pixelShaderHash", m_pixelShaderHash);

	s >> MemberStaticArray< CBufferDesc, 3, MemberComposite< CBufferDesc > >(L"vertexCBuffers", m_vertexCBuffers);
	s >> MemberStaticArray< CBufferDesc, 3, MemberComposite< CBufferDesc > >(L"pixelCBuffers", m_pixelCBuffers);

	s >> MemberAlignedVector< ResourceBindingDesc, MemberComposite< ResourceBindingDesc > >(L"vertexTextureBindings", m_vertexTextureBindings);
	s >> MemberAlignedVector< ResourceBindingDesc, MemberComposite< ResourceBindingDesc > >(L"pixelTextureBindings", m_pixelTextureBindings);

	s >> MemberAlignedVector< ResourceBindingDesc, MemberComposite< ResourceBindingDesc > >(L"vertexStructBufferBindings", m_vertexStructBufferBindings);
	s >> MemberAlignedVector< ResourceBindingDesc, MemberComposite< ResourceBindingDesc > >(L"pixelStructBufferBindings", m_pixelStructBufferBindings);

	s >> MemberAlignedVector< D3D11_SAMPLER_DESC, MemberD3D11_SAMPLER_DESC >(L"vertexSamplers", m_vertexSamplers);
	s >> MemberAlignedVector< D3D11_SAMPLER_DESC, MemberD3D11_SAMPLER_DESC >(L"pixelSamplers", m_pixelSamplers);

	s >> MemberAlignedVector< ParameterDesc, MemberComposite< ParameterDesc > >(L"parameters", m_parameters);
	s >> Member< uint32_t >(L"parameterScalarSize", m_parameterScalarSize);
	s >> Member< uint32_t >(L"parameterTextureSize", m_parameterTextureSize);
	s >> Member< uint32_t >(L"parameterStructBufferSize", m_parameterStructBufferSize);

	s >> MemberD3D11_RASTERIZER_DESC(L"d3dRasterizerDesc", m_d3dRasterizerDesc);
	s >> MemberD3D11_DEPTH_STENCIL_DESC(L"d3dDepthStencilDesc", m_d3dDepthStencilDesc);
	s >> MemberD3D11_BLEND_DESC(L"d3dBlendDesc", m_d3dBlendDesc);
	s >> Member< uint32_t >(L"stencilReference", m_stencilReference);
}

void ProgramResourceDx11::ParameterDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"count", count);
}

void ProgramResourceDx11::ParameterMappingDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"cbufferOffset", cbufferOffset);
	s >> Member< uint32_t >(L"parameterOffset", parameterOffset);
	s >> Member< uint32_t >(L"parameterCount", parameterCount);
}

void ProgramResourceDx11::CBufferDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", size);
	s >> MemberAlignedVector< ParameterMappingDesc, MemberComposite< ParameterMappingDesc > >(L"parameters", parameters);
}

void ProgramResourceDx11::ResourceBindingDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"bindPoint", bindPoint);
	s >> Member< uint32_t >(L"parameterOffset", parameterOffset);
}

	}
}
