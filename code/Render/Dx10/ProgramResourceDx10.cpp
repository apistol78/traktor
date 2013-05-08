#include "Render/Dx10/ProgramResourceDx10.h"
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

class MemberID3D10Blob : public MemberComplex
{
public:
	MemberID3D10Blob(const wchar_t* const name, ComRef< ID3D10Blob >& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		uint8_t blob[65535];
		uint32_t blobSize;

		if (s.getDirection() == ISerializer::SdRead)
		{
			blobSize = sizeof(blob);
			s >> Member< void* >(getName(), blob, blobSize);

			D3D10CreateBlob(blobSize, &m_ref.getAssign());
			std::memcpy(m_ref->GetBufferPointer(), blob, blobSize);
		}
		else	// SdWrite
		{
			blobSize = m_ref ? m_ref->GetBufferSize() : 0;
			T_ASSERT (blobSize < sizeof(blob));

			if (m_ref)
				std::memcpy(blob, m_ref->GetBufferPointer(), blobSize);

			s >> Member< void* >(getName(), blob, blobSize);
		}
	}

private:
	ComRef< ID3D10Blob >& m_ref;
};

class MemberD3D10_RASTERIZER_DESC : public MemberComplex
{
public:
	MemberD3D10_RASTERIZER_DESC(const wchar_t* const name, D3D10_RASTERIZER_DESC& ref)
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
	D3D10_RASTERIZER_DESC& m_ref;
};

class MemberD3D10_DEPTH_STENCIL_DESC : public MemberComplex
{
public:
	MemberD3D10_DEPTH_STENCIL_DESC(const wchar_t* const name, D3D10_DEPTH_STENCIL_DESC& ref)
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
	D3D10_DEPTH_STENCIL_DESC& m_ref;
};

class MemberD3D10_BLEND_DESC : public MemberComplex
{
public:
	MemberD3D10_BLEND_DESC(const wchar_t* const name, D3D10_BLEND_DESC& ref)
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
	D3D10_BLEND_DESC& m_ref;
};

class MemberD3D10_SAMPLER_DESC : public MemberComplex
{
public:
	MemberD3D10_SAMPLER_DESC(const wchar_t* const name, D3D10_SAMPLER_DESC& ref)
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
	D3D10_SAMPLER_DESC& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx10", 0, ProgramResourceDx10, ProgramResource)

ProgramResourceDx10::ProgramResourceDx10()
:	m_stencilReference(0)
{
	std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	std::memset(&m_d3dDepthStencilDesc, 0, sizeof(m_d3dDepthStencilDesc));
	std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
}

void ProgramResourceDx10::serialize(ISerializer& s)
{
	s >> MemberID3D10Blob(L"vertexShader", m_vertexShader);
	s >> MemberID3D10Blob(L"pixelShader", m_pixelShader);
	s >> MemberD3D10_RASTERIZER_DESC(L"d3dRasterizerDesc", m_d3dRasterizerDesc);
	s >> MemberD3D10_DEPTH_STENCIL_DESC(L"d3dDepthStencilDesc", m_d3dDepthStencilDesc);
	s >> MemberD3D10_BLEND_DESC(L"d3dBlendDesc", m_d3dBlendDesc);
	s >> Member< uint32_t >(L"stencilReference", m_stencilReference);
	s >> MemberStlMap<
		std::wstring,
		D3D10_SAMPLER_DESC,
		MemberStlPair<
			std::wstring,
			D3D10_SAMPLER_DESC,
			Member< std::wstring >,
			MemberD3D10_SAMPLER_DESC
		>
	>(L"d3dVertexSamplers", m_d3dVertexSamplers);
	s >> MemberStlMap<
		std::wstring,
		D3D10_SAMPLER_DESC,
		MemberStlPair<
			std::wstring,
			D3D10_SAMPLER_DESC,
			Member< std::wstring >,
			MemberD3D10_SAMPLER_DESC
		>
	>(L"d3dPixelSamplers", m_d3dPixelSamplers);
}

	}
}
