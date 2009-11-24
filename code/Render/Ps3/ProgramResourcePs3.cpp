#include "Render/Ps3/ProgramResourcePs3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberBin : public MemberComplex
{
public:
	MemberBin(const std::wstring& name, CGCbin*& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdRead)
		{
			uint32_t size;
			if (!(s >> Member< uint32_t >(L"size", size)))
				return false;

			if (size > 0)
			{
				m_ref = sceCgcNewBin();
				if (sceCgcStoreBinData(m_ref, 0, size) != SCECGC_OK)
					return false;

				s >> Member< void* >(L"data", sceCgcGetBinData(m_ref), size);
			}
			else
				m_ref = 0;
		}
		else	// SdWrite
		{
			uint32_t size = m_ref ? sceCgcGetBinSize(m_ref) : 0;
			if (!(s >> Member< uint32_t >(L"size", size)))
				return false;

			if (size > 0)
				s >> Member< void* >(L"data", sceCgcGetBinData(m_ref), size);
		}
		return true;
	}

private:
	CGCbin*& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourcePs3", 0, ProgramResourcePs3, ProgramResource)

ProgramResourcePs3::ProgramResourcePs3()
:	m_vertexShaderBin(0)
,	m_pixelShaderBin(0)
{
}

ProgramResourcePs3::ProgramResourcePs3(
	CGCbin* vertexShaderBin,
	CGCbin* pixelShaderBin
)
:	m_vertexShaderBin(vertexShaderBin)
,	m_pixelShaderBin(pixelShaderBin)
{
}

ProgramResourcePs3::~ProgramResourcePs3()
{
	if (m_vertexShaderBin)
		sceCgcDeleteBin(m_vertexShaderBin);
	if (m_pixelShaderBin)
		sceCgcDeleteBin(m_pixelShaderBin);
}

bool ProgramResourcePs3::serialize(ISerializer& s)
{
	s >> MemberBin(L"vertexProgramBin", m_vertexShaderBin);
	s >> MemberBin(L"pixelProgramBin", m_pixelShaderBin);
	return true;
}

	}
}
