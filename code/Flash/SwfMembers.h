#ifndef traktor_flash_SwfMembers_H
#define traktor_flash_SwfMembers_H

#include "Core/Serialization/MemberComplex.h"
#include "Flash/ColorTransform.h"

namespace traktor
{
	namespace flash
	{

/*! \brief Color transform member.
 * \ingroup Flash
 */
class MemberColorTransform : public MemberComplex
{
public:
	MemberColorTransform(const wchar_t* const name, ColorTransform& ref);

	virtual void serialize(ISerializer& s) const;

private:
	ColorTransform& m_ref;
};

	}
}

#endif	// traktor_flash_SwfMembers_H
