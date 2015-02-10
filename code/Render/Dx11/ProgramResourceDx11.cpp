#include "Render/Dx11/Blob.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Shader/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

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
		uint8_t blob[131072];
		uint32_t blobSize;

		if (s.getDirection() == ISerializer::SdRead)
		{
			blobSize = sizeof(blob);
			s >> Member< void* >(getName(), blob, blobSize);

			m_ref = Blob::create(blobSize);
			if (!m_ref)
			{
				s.failure();
				return;
			}

			std::memcpy(m_ref->getData(), blob, blobSize);
		}
		else	// SdWrite
		{
			blobSize = m_ref ? m_ref->getSize() : 0;
			T_ASSERT (blobSize < sizeof(blob));

			if (m_ref)
				std::memcpy(blob, m_ref->getData(), blobSize);

			s >> Member< void* >(getName(), blob, blobSize);
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
		uint32_t size = sizeof(m_ref);
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
		uint32_t size = sizeof(m_ref);
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
		uint32_t size = sizeof(m_ref);
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
		uint32_t size = sizeof(m_ref);
		s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_SAMPLER_DESC& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx11", 0, ProgramResourceDx11, ProgramResource)

ProgramResourceDx11::ProgramResourceDx11()
:	m_vertexShaderHash(0)
,	m_pixelShaderHash(0)
,	m_parameterScalarSize(0)
,	m_parameterTextureSize(0)
,	m_stencilReference(0)
{
	std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	std::memset(&m_d3dDepthStencilDesc, 0, sizeof(m_d3dDepthStencilDesc));
	std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
}

void ProgramResourceDx11::serialize(ISerializer& s)
{
	s >> MemberBlob(L"vertexShader", m_vertexShader);
	s >> MemberBlob(L"pixelShader", m_pixelShader);
	
	s >> Member< uint32_t >(L"vertexShaderHash", m_vertexShaderHash);
	s >> Member< uint32_t >(L"pixelShaderHash", m_pixelShaderHash);

	s >> MemberStaticArray< CBufferDesc, 3, MemberComposite< CBufferDesc > >(L"vertexCBuffers", m_vertexCBuffers);
	s >> MemberStaticArray< CBufferDesc, 3, MemberComposite< CBufferDesc > >(L"pixelCBuffers", m_pixelCBuffers);

	s >> MemberStlVector< TextureBindingDesc, MemberComposite< TextureBindingDesc > >(L"vertexTextureBindings", m_vertexTextureBindings);
	s >> MemberStlVector< TextureBindingDesc, MemberComposite< TextureBindingDesc > >(L"pixelTextureBindings", m_pixelTextureBindings);

	s >> MemberStlVector< D3D11_SAMPLER_DESC, MemberD3D11_SAMPLER_DESC >(L"vertexSamplers", m_vertexSamplers);
	s >> MemberStlVector< D3D11_SAMPLER_DESC, MemberD3D11_SAMPLER_DESC >(L"pixelSamplers", m_pixelSamplers);

	s >> MemberStlVector< ParameterDesc, MemberComposite< ParameterDesc > >(L"parameters", m_parameters);
	s >> Member< uint32_t >(L"parameterScalarSize", m_parameterScalarSize);
	s >> Member< uint32_t >(L"parameterTextureSize", m_parameterTextureSize);

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
	s >> MemberStlVector< ParameterMappingDesc, MemberComposite< ParameterMappingDesc > >(L"parameters", parameters);
}

void ProgramResourceDx11::TextureBindingDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"bindPoint", bindPoint);
	s >> Member< uint32_t >(L"parameterOffset", parameterOffset);
}

	}
}
