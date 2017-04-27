/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ReflectionMember_H
#define traktor_ReflectionMember_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Reflected member base class.
 * \ingroup Core
 */
class T_DLLCLASS ReflectionMember : public Object
{
	T_RTTI_CLASS;

public:
	const wchar_t* getName() const;

	virtual bool replace(const ReflectionMember* source) = 0;

protected:
	ReflectionMember(const wchar_t* name);

private:
	const wchar_t* m_name;
};

}

#endif	// traktor_ReflectionMember_H
