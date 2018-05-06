/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_ResourceHandle_H
#define traktor_resource_ResourceHandle_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Resource handle base class.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceHandle : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Replace resource object.
	 *
	 * \param object New resource object.
	 */
	void replace(Object* object) { m_object = object; }

	/*! \brief Get resource object.
	 *
	 * \return Resource object.
	 */
	Object* get() const { return m_object; }

	/*! \brief Flush resource object.
	 */
	void flush() { m_object = 0; }

protected:
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResourceHandle_H
