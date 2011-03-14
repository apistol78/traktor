#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Shader/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberID3DBlob : public MemberComplex
{
public:
	MemberID3DBlob(const std::wstring& name, ComRef< ID3DBlob >& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		uint8_t blob[65535];
		uint32_t blobSize;

		if (s.getDirection() == ISerializer::SdRead)
		{
			blobSize = sizeof(blob);
			if (!(s >> Member< void* >(getName(), blob, blobSize)))
				return false;

			D3DCreateBlob(blobSize, &m_ref.getAssign());
			std::memcpy(m_ref->GetBufferPointer(), blob, blobSize);
		}
		else	// SdWrite
		{
			blobSize = m_ref->GetBufferSize();
			T_ASSERT (blobSize < sizeof(blob));
			std::memcpy(blob, m_ref->GetBufferPointer(), blobSize);

			if (!(s >> Member< void* >(getName(), blob, blobSize)))
				return false;
		}

		return true;
	}

private:
	ComRef< ID3DBlob >& m_ref;
};

class MemberD3D11_RASTERIZER_DESC : public MemberComplex
{
public:
	MemberD3D11_RASTERIZER_DESC(const std::wstring& name, D3D11_RASTERIZER_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		uint32_t size = sizeof(m_ref);
		return s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_RASTERIZER_DESC& m_ref;
};

class MemberD3D11_DEPTH_STENCIL_DESC : public MemberComplex
{
public:
	MemberD3D11_DEPTH_STENCIL_DESC(const std::wstring& name, D3D11_DEPTH_STENCIL_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		uint32_t size = sizeof(m_ref);
		return s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_DEPTH_STENCIL_DESC& m_ref;
};

class MemberD3D11_BLEND_DESC : public MemberComplex
{
public:
	MemberD3D11_BLEND_DESC(const std::wstring& name, D3D11_BLEND_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		uint32_t size = sizeof(m_ref);
		return s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_BLEND_DESC& m_ref;
};

class MemberD3D11_SAMPLER_DESC : public MemberComplex
{
public:
	MemberD3D11_SAMPLER_DESC(const std::wstring& name, D3D11_SAMPLER_DESC& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		uint32_t size = sizeof(m_ref);
		return s >> Member< void* >(getName(), (void*)&m_ref, size);
	}

private:
	D3D11_SAMPLER_DESC& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx11", 0, ProgramResourceDx11, ProgramResource)

bool ProgramResourceDx11::serialize(ISerializer& s)
{
	s >> MemberID3DBlob(L"vertexShader", m_vertexShader);
	s >> MemberID3DBlob(L"pixelShader", m_pixelShader);
	s >> MemberD3D11_RASTERIZER_DESC(L"d3dRasterizerDesc", m_d3dRasterizerDesc);
	s >> MemberD3D11_DEPTH_STENCIL_DESC(L"d3dDepthStencilDesc", m_d3dDepthStencilDesc);
	s >> MemberD3D11_BLEND_DESC(L"d3dBlendDesc", m_d3dBlendDesc);
	s >> Member< uint32_t >(L"stencilReference", m_stencilReference);
	s >> MemberStlMap<
		std::wstring,
		D3D11_SAMPLER_DESC,
		MemberStlPair<
			std::wstring,
			D3D11_SAMPLER_DESC,
			Member< std::wstring >,
			MemberD3D11_SAMPLER_DESC
		>
	>(L"d3dVertexSamplers", m_d3dVertexSamplers);
	s >> MemberStlMap<
		std::wstring,
		D3D11_SAMPLER_DESC,
		MemberStlPair<
			std::wstring,
			D3D11_SAMPLER_DESC,
			Member< std::wstring >,
			MemberD3D11_SAMPLER_DESC
		>
	>(L"d3dPixelSamplers", m_d3dPixelSamplers);
	return true;
}

	}
}
