/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IPropertyValue_H
#define traktor_IPropertyValue_H

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Property value base.
 * \ingroup Core
 */
class T_DLLCLASS IPropertyValue : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const = 0;

	virtual Ref< IPropertyValue > clone() const = 0;
};

/*!
 * \ingroup Core
 */
template< typename ValueType >
struct PropertyTrait {};

}

#endif	// traktor_IPropertyValue_H
