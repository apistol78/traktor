#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"

namespace traktor
{

MemberBitMask::MemberBitMask(const wchar_t* const name, uint32_t& bm, const Bit* bits)
:	MemberComplex(name, true)
,	m_bits(bits)
,	m_bm(bm)
{
}

void MemberBitMask::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::SdRead)
		m_bm = 0;
	for (int i = 0; m_bits[i].id; ++i)
	{
		bool bv = bool((m_bm & m_bits[i].mask) != 0);
		s >> Member< bool >(m_bits[i].id, bv);
		m_bm |= bv ? m_bits[i].mask : 0;
	}
}

}
