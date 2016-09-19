#ifndef traktor_flash_SwfMembers_H
#define traktor_flash_SwfMembers_H

#include "Core/Serialization/MemberComplex.h"
#include "Flash/SwfTypes.h"

namespace traktor
{
	namespace flash
	{

class MemberSwfCxTransform : public MemberComplex
{
public:
	MemberSwfCxTransform(const wchar_t* const name, SwfCxTransform& ref);

	virtual void serialize(ISerializer& s) const;

private:
	SwfCxTransform& m_ref;
};

	}
}

#endif	// traktor_flash_SwfMembers_H
