/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Editor/ConvertSwf.h"

#include "Spark/Swf/SwfMovieFactory.h"
#include "Spark/Swf/SwfReader.h"

namespace traktor::spark
{

Ref< Movie > convertSwf(const db::Instance* sourceInstance, IStream* sourceStream)
{
	Ref< SwfReader > swf = new SwfReader(sourceStream);
	return SwfMovieFactory().createMovie(swf);
}

}
