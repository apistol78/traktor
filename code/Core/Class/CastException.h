/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_CastException_H
#define traktor_CastException_H

#include <exception>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS CastException : public std::exception
{
public:
	CastException(const char* what_);

	virtual const char* what() const throw();

private:
	const char* m_what;
};

}

#endif	// traktor_CastException_H
