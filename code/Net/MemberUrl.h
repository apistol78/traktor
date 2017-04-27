/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_MemberUrl_H
#define traktor_net_MemberUrl_H

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Net/Url.h"

namespace traktor
{
	namespace net
	{

/*! \brief URL serialization member.
 * \ingroup Net
 */
class MemberUrl : public MemberComplex
{
public:
	typedef Url value_type;

	MemberUrl(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}
	
	virtual void serialize(ISerializer& s) const
	{
		std::wstring url;
		if (s.getDirection() == ISerializer::SdRead)
		{
			s >> Member< std::wstring >(getName(), url);
			m_ref = Url(url);
		}
		else
		{
			url = m_ref.getString();
			s >> Member< std::wstring >(getName(), url);
		}
	}
	
private:
	value_type& m_ref;
};

	}
}

#endif	// traktor_net_MemberUrl_H
