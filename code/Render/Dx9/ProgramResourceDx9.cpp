#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Dx9/ProgramResourceDx9.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberID3DXBuffer : public MemberComplex
{
public:
	MemberID3DXBuffer(const wchar_t* const name, ComRef< ID3DXBuffer >& ref)
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

			if (blobSize > 0)
			{
				D3DXCreateBuffer(blobSize, &m_ref.getAssign());
				std::memcpy(m_ref->GetBufferPointer(), blob, blobSize);
			}
			else
				m_ref = 0;
		}
		else	// SdWrite
		{
			if (m_ref)
			{
				blobSize = m_ref->GetBufferSize();
				T_ASSERT (blobSize < sizeof(blob));
				std::memcpy(blob, m_ref->GetBufferPointer(), blobSize);
			}
			else
				blobSize = 0;

			if (!(s >> Member< void* >(getName(), blob, blobSize)))
				return false;
		}

		return true;
	}

private:
	ComRef< ID3DXBuffer >& m_ref;
};

class MemberProgramScalar : public MemberComplex
{
public:
	MemberProgramScalar(const wchar_t* const name, ProgramScalar& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint16_t >(L"registerIndex", m_ref.registerIndex);
		s >> Member< uint16_t >(L"registerCount", m_ref.registerCount);
		s >> Member< uint16_t >(L"offset", m_ref.offset);
		s >> Member< uint16_t >(L"length", m_ref.length);
		return true;
	}

private:
	ProgramScalar& m_ref;
};

class MemberProgramSampler : public MemberComplex
{
public:
	MemberProgramSampler(const wchar_t* const name, ProgramSampler& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint16_t >(L"texture", m_ref.texture);
		s >> Member< uint16_t >(L"stage", m_ref.stage);
		return true;
	}

private:
	ProgramSampler& m_ref;
};

class MemberProgramTexture : public MemberComplex
{
public:
	MemberProgramTexture(const wchar_t* const name, ProgramTexture& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint16_t >(L"texture", m_ref.texture);
		s >> Member< uint16_t >(L"sizeIndex", m_ref.sizeIndex);
		return true;
	}

private:
	ProgramTexture& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx9", 0, ProgramResourceDx9, ProgramResource)

ProgramResourceDx9::ProgramResourceDx9()
:	m_vertexShaderHash(0)
,	m_pixelShaderHash(0)
,	m_scalarParameterDataSize(0)
,	m_textureParameterDataSize(0)
{
}

bool ProgramResourceDx9::serialize(ISerializer& s)
{
	s >> MemberID3DXBuffer(L"vertexShader", m_vertexShader);
	s >> MemberID3DXBuffer(L"pixelShader", m_pixelShader);
	s >> Member< uint32_t >(L"vertexShaderHash", m_vertexShaderHash);
	s >> Member< uint32_t >(L"pixelShaderHash", m_pixelShaderHash);
	s >> MemberAlignedVector< ProgramScalar, MemberProgramScalar >(L"vertexScalars", m_vertexScalars);
	s >> MemberAlignedVector< ProgramScalar, MemberProgramScalar >(L"pixelScalars", m_pixelScalars);
	s >> MemberAlignedVector< ProgramTexture , MemberProgramTexture >(L"vertexTextures", m_vertexTextures);
	s >> MemberAlignedVector< ProgramTexture, MemberProgramTexture >(L"pixelTextures", m_pixelTextures);
	s >> MemberAlignedVector< ProgramSampler, MemberProgramSampler >(L"vertexSamplers", m_vertexSamplers);
	s >> MemberAlignedVector< ProgramSampler, MemberProgramSampler >(L"pixelSamplers", m_pixelSamplers);
	s >> MemberStlMap< std::wstring, uint32_t >(L"scalarParameterMap", m_scalarParameterMap);
	s >> MemberStlMap< std::wstring, uint32_t >(L"textureParameterMap", m_textureParameterMap);
	s >> Member< uint32_t >(L"scalarParameterDataSize", m_scalarParameterDataSize);
	s >> Member< uint32_t >(L"textureParameterDataSize", m_textureParameterDataSize);
	s >> MemberComposite< StateBlockDx9 >(L"state", m_state);
	return true;
}

	}
}
