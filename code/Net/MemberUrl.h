#ifndef traktor_net_MemberUrl_H
#define traktor_net_MemberUrl_H

#include "Core/Serialization/MemberComplex.h"
#include "Net/Url.h"

namespace traktor
{
	namespace net
	{

class MemberUrl : public MemberComplex
{
public:
	typedef Url value_type;

	MemberUrl(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}
	
	virtual void serialize(Serializer& s) const
	{
		std::wstring url;
		if (s.getDirection() == Serializer::SdRead)
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
