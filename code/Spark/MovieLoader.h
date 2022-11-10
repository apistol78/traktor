/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/IMovieLoader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace spark
	{

class T_DLLCLASS MovieLoader : public IMovieLoader
{
	T_RTTI_CLASS;

public:
	MovieLoader();

	void setCacheDirectory(const std::wstring& cacheDirectory);

	void setMerge(bool merge);

	void setTriangulate(bool triangulate);

	virtual Ref< IHandle > loadAsync(const std::wstring& url) const override final;

	virtual Ref< Movie > load(const std::wstring& url) const override final;

private:
	std::wstring m_cacheDirectory;
	bool m_merge;
	bool m_triangulate;
};

	}
}

