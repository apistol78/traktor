#include "Render/Dx9/ProgramResourceDx9.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberAggregate.h"

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

	virtual bool serialize(Serializer& s) const
	{
		uint8_t blob[65535];
		uint32_t blobSize;

		if (s.getDirection() == Serializer::SdRead)
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ProgramResourceDx9", ProgramResourceDx9, ProgramResource)

bool ProgramResourceDx9::serialize(Serializer& s)
{
	if (!ProgramResource::serialize(s))
		return false;

	s >> MemberID3DXBuffer(L"vertexShader", m_vertexShader);
	s >> MemberID3DXBuffer(L"pixelShader", m_pixelShader);
	s >> MemberAggregate< StateBlockDx9 >(L"state", m_state);
	return true;
}

	}
}
