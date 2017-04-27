/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberComposite_H
#define traktor_MemberComposite_H

#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \brief Composite member.
 * \ingroup Core
 */
template < typename Class, bool Compound = true >	
class MemberComposite : public MemberComplex
{
public:
	typedef Class value_type;
	
	MemberComposite(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, Compound)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		m_ref.serialize(s);
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberComposite_H
