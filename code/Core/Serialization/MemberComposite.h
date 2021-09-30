#pragma once

#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! Composite member.
 * \ingroup Core
 */
template < typename Class, bool Compound = true >
class MemberComposite : public MemberComplex
{
public:
	typedef Class value_type;

	explicit MemberComposite(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, Compound)
	,	m_ref(ref)
	{
	}

	explicit MemberComposite(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberComplex(name, Compound, attributes)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		m_ref.serialize(s);
	}

private:
	value_type& m_ref;
};

}

