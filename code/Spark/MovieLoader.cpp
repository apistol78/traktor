/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/MovieResult.h"
#include "Spark/Optimizer.h"
#include "Spark/Sprite.h"
#include "Spark/Swf/SwfMovieFactory.h"
#include "Spark/Swf/SwfReader.h"
#include "Net/UrlConnection.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MovieLoader", MovieLoader, IMovieLoader)

MovieLoader::MovieLoader()
:	m_merge(false)
,	m_triangulate(false)
{
}

void MovieLoader::setCacheDirectory(const std::wstring& cacheDirectory)
{
	m_cacheDirectory = cacheDirectory;
}

void MovieLoader::setMerge(bool merge)
{
	m_merge = merge;
}

void MovieLoader::setTriangulate(bool triangulate)
{
	m_triangulate = triangulate;
}

Ref< MovieResult > MovieLoader::loadAsync(const std::wstring& url) const
{
	Ref< MovieResult > result = new MovieResult();
	JobManager::getInstance().add([=](){
		T_ANONYMOUS_VAR(Ref< const MovieLoader >)(this);
		Ref< Movie > movie;

		Ref< net::UrlConnection > connection = net::UrlConnection::open(net::Url(url));
		if (!connection)
		{
			result->fail();
			return;
		}

		Ref< IStream > s = connection->getStream();
		T_ASSERT(s);

		const std::wstring ext = toLower(traktor::Path(url).getExtension());

		// Try to load image and embed into a movie first, if extension
		// not supported then this fail quickly.
		Ref< drawing::Image > image = drawing::Image::load(s, ext);
		if (image)
			movie = SwfMovieFactory().createMovieFromImage(image);
		else
		{
			SwfReader swfReader(s);
			movie = SwfMovieFactory().createMovie(&swfReader);
		}

		if (!movie)
		{
			result->fail();
			return;
		}

		if (m_merge)
			movie = Optimizer().merge(movie);

		if (m_triangulate)
			Optimizer().triangulate(movie, false);

		result->succeed(movie);
	});
	return result;
}

Ref< Movie > MovieLoader::load(const std::wstring& url) const
{
	Ref< MovieResult > handle = loadAsync(url);
	return handle ? handle->get() : nullptr;
}

}
