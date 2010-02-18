#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"

namespace traktor
{

MemberBitMask::MemberBitMask(const std::wstring& name, uint32_t& bm, const Bit* bits)
:	MemberComplex(name, true)
,	m_bits(bits)
,	m_bm(bm)
{
}

bool MemberBitMask::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::SdRead)
		m_bm = 0;
	for (int i = 0; m_bits[i].id; ++i)
	{
		bool bv = bool((m_bm & m_bits[i].mask) != 0);
		if (!(s >> Member< bool >(m_bits[i].id, bv)))
			return false;
		m_bm |= bv ? m_bits[i].mask : 0;
	}
	return true;
}

}
