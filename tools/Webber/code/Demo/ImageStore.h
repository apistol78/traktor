/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"

namespace traktor
{

class IStream;

}

namespace traktor::demo
{

/*! Uploaded images, one file each in a directory; shared by all sessions.
 * \ingroup Webber
 *
 * Images are served from "/images/<name>"; only PNG, JPEG, GIF and WebP are accepted.
 */
class ImageStore : public Object
{
	T_RTTI_CLASS;

public:
	explicit ImageStore(const Path& directory);

	/*! Store image; returns URI relative to site root, empty if data isn't a supported image. */
	std::wstring store(const AlignedVector< uint8_t >& data);

	/*! Open image by URI as requested from the site root; null if there is no such image. */
	Ref< IStream > open(const std::wstring& uri) const;

private:
	Path m_directory;
};

}
