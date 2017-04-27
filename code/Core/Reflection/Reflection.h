/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Reflection_H
#define traktor_Reflection_H

#include "Core/Reflection/RfmCompound.h"

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

/*! \brief Object data reflection.
 * \ingroup Core
 *
 * Using serialization mechanism to create a
 * reflection structure which can be queried
 * or altered in a type independent manor.
 *
 * \note
 * The reflection is shallow, thus only reflect
 * the first level of objects. Deeper level objects
 * are exposed as a RfmObject member descriptor.
 */
class T_DLLCLASS Reflection : public RfmCompound
{
	T_RTTI_CLASS;

public:
	/*! \brief Create a reflection model from object.
	 *
	 * \param object Source object.
	 * \return Reflection model.
	 */
	static Ref< Reflection > create(const ISerializable* object);

	/*! \brief Apply changes on model back onto object.
	 *
	 * \param object Target object.
	 * \return True if changes successfully applied.
	 */
	bool apply(ISerializable* object) const;

	/*! \brief Use model to create a clone of the source object.
	 *
	 * \return Clone of source object.
	 */
	Ref< ISerializable > clone() const;

private:
	const TypeInfo& m_objectType;

	Reflection(const TypeInfo& objectType);
};

}

#endif	// traktor_Reflection_H
