#pragma once

#include "Core/Serialization/MemberComplex.h"
#include "Spark/ColorTransform.h"

namespace traktor
{
	namespace spark
	{

/*! Color transform member.
 * \ingroup Spark
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

