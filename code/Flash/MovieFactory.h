/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MovieFactory_H
#define traktor_flash_MovieFactory_H

#include <map>
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
	namespace drawing
	{
	
class Image;

	}

	namespace flash
	{

class SwfReader;
class Movie;
class Tag;

/*! \brief Flash movie factory.
 * \ingroup Flash
 */
class T_DLLCLASS MovieFactory : public Object
{
	T_RTTI_CLASS;

public:
	MovieFactory(bool includeAS);

	Ref< Movie > createMovie(SwfReader* swf) const;

	Ref< Movie > createMovieFromImage(const drawing::Image* image) const;

private:
	bool m_includeAS;
	std::map< uint16_t, Ref< Tag > > m_tagReaders;
};

	}
}

#endif	// traktor_flash_MovieFactory_H
