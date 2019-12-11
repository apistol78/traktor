#pragma once

#include "Core/Serialization/Attribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Member type attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributeType : public Attribute
{
	T_RTTI_CLASS;

public:
	AttributeType(const TypeInfo& memberType);

	const TypeInfo& getMemberType() const;

private:
	const TypeInfo& m_memberType;
};

}

