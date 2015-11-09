#include "Render/Ps4/BlobPs4.h"
#include "Render/Ps4/ProgramResourcePs4.h"
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

class MemberBlobPs4 : public MemberComplex
{
public:
	MemberBlobPs4(const wchar_t* const name, Ref< BlobPs4 >& ref)
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

			m_ref = BlobPs4::create(blobSize);
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
	Ref< BlobPs4 >& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourcePs4", 0, ProgramResourcePs4, ProgramResource)

ProgramResourcePs4::ProgramResourcePs4()
{
}

void ProgramResourcePs4::serialize(ISerializer& s)
{
	s >> MemberBlobPs4(L"vertexShader", m_vertexShader);
	s >> MemberBlobPs4(L"pixelShader", m_pixelShader);
}

	}
}
