#ifndef traktor_flash_SwfMembers_H
#define traktor_flash_SwfMembers_H

#include "Core/Serialization/MemberComplex.h"
#include "Flash/SwfTypes.h"

namespace traktor
{
	namespace flash
	{

class MemberSwfColor : public MemberComplex
{
public:
	MemberSwfColor(const wchar_t* const name, SwfColor& ref);

	virtual void serialize(ISerializer& s) const;

private:
	SwfColor& m_ref;
};

class MemberSwfCxTransform : public MemberComplex
{
public:
	MemberSwfCxTransform(const wchar_t* const name, SwfCxTransform& ref);

	virtual void serialize(ISerializer& s) const;

private:
	SwfCxTransform& m_ref;
};

class MemberSwfRect : public MemberComplex
{
public:
	MemberSwfRect(const wchar_t* const name, SwfRect& ref);

	virtual void serialize(ISerializer& s) const;

private:
	SwfRect& m_ref;
};

	}
}

#endif	// traktor_flash_SwfMembers_H
