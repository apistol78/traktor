/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Movie;
class MovieResult;

class T_DLLCLASS IMovieLoader : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< MovieResult > loadAsync(const std::wstring& url) const = 0;

	virtual Ref< Movie > load(const std::wstring& url) const = 0;
};

}
