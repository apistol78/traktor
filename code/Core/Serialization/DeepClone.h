/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_DeepClone_H
#define traktor_DeepClone_H

#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! \brief Clone object.
 * \ingroup Core
 *
 * Creates a clone of an object through
 * serialization.
 */
class T_DLLCLASS DeepClone : public Object
{
	T_RTTI_CLASS;

public:
	DeepClone(const ISerializable* source);

	/*! \brief Create new instance of source object. */
	Ref< ISerializable > create();

	/*! \brief Create new instance of source object. */
	template < typename T >
	Ref< T > create()
	{
		return dynamic_type_cast< T* >(create());
	}

private:
	AlignedVector< uint8_t > m_copy;
};

}

#endif	// traktor_DeepClone_H
