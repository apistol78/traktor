/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashMovieFactory_H
#define traktor_flash_FlashMovieFactory_H

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
class FlashMovie;
class FlashTag;

/*! \brief Flash movie factory.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieFactory : public Object
{
	T_RTTI_CLASS;

public:
	FlashMovieFactory(bool includeAS);

	Ref< FlashMovie > createMovie(SwfReader* swf) const;

	Ref< FlashMovie > createMovieFromImage(const drawing::Image* image) const;

private:
	bool m_includeAS;
	std::map< uint16_t, Ref< FlashTag > > m_tagReaders;
};

	}
}

#endif	// traktor_flash_FlashMovieFactory_H
