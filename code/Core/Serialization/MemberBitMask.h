#pragma once

#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Bit mask member.
 * \ingroup Core
 */
class T_DLLCLASS MemberBitMask : public MemberComplex
{
public:
	struct Bit
	{
		const wchar_t* const id;
		uint32_t mask;
	};

	explicit MemberBitMask(const wchar_t* const name, uint32_t& bm, const Bit* bits);

	virtual void serialize(ISerializer& s) const override final;

private:
	const Bit* m_bits;
	uint32_t& m_bm;
};

}

