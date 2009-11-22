#include "Render/Dx9/ProgramResourceDx9.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberID3DXBuffer : public MemberComplex
{
public:
	MemberID3DXBuffer(const std::wstring& name, ComRef< ID3DXBuffer >& ref)
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

			D3DXCreateBuffer(blobSize, &m_ref.getAssign());
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
	ComRef< ID3DXBuffer >& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceDx9", 0, ProgramResourceDx9, ProgramResource)

ProgramResourceDx9::ProgramResourceDx9()
:	m_vertexShaderHash(0)
,	m_pixelShaderHash(0)
{
}

bool ProgramResourceDx9::serialize(ISerializer& s)
{
	if (!ProgramResource::serialize(s))
		return false;

	s >> MemberID3DXBuffer(L"vertexShader", m_vertexShader);
	s >> MemberID3DXBuffer(L"pixelShader", m_pixelShader);
	s >> Member< uint32_t >(L"vertexShaderHash", m_vertexShaderHash);
	s >> Member< uint32_t >(L"pixelShaderHash", m_pixelShaderHash);
	s >> MemberComposite< StateBlockDx9 >(L"state", m_state);

	return true;
}

	}
}
