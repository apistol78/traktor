/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::spark
{

class SwfReader;
class Movie;
class Tag;

/*! Flash movie factory.
 * \ingroup Spark
 */
class T_DLLCLASS SwfMovieFactory : public Object
{
	T_RTTI_CLASS;

public:
	SwfMovieFactory();

	Ref< Movie > createMovie(SwfReader* swf) const;

	Ref< Movie > createMovieFromImage(const drawing::Image* image) const;

private:
	SmallMap< uint16_t, Ref< Tag > > m_tagReaders;
};

}
