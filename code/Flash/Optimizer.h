/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Optimizer_H
#define traktor_flash_Optimizer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Movie;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS Optimizer : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Movie > merge(const Movie* movie) const;

	void triangulate(Movie* movie, bool discardPaths) const;
};

	}
}

#endif	// traktor_flash_Optimizer_H
