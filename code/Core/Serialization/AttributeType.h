/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_AttributeType_H
#define traktor_AttributeType_H

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

/*! \brief Member type attribute.
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

#endif	// traktor_AttributeType_H
