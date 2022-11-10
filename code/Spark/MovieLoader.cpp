/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/Optimizer.h"
#include "Spark/Sprite.h"
#include "Spark/Swf/SwfMovieFactory.h"
#include "Spark/Swf/SwfReader.h"
#include "Net/UrlConnection.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class MovieLoaderHandle : public IMovieLoader::IHandle
{
public:
	MovieLoaderHandle(const std::wstring& url, const std::wstring& cacheDirectory, bool merge, bool triangulate)
	:	m_url(url)
	,	m_cacheDirectory(cacheDirectory)
	,	m_merge(merge)
	,	m_triangulate(triangulate)
	{
		m_job = JobManager::getInstance().add([this](){
			loader();
		});
	}

	virtual bool wait() override final
	{
		return m_job ? m_job->wait() : true;
	}

	virtual bool ready() override final
	{
		return m_job ? m_job->stopped() : true;
	}

	virtual bool succeeded() override final
	{
		return wait() ? (m_movie != 0) : false;
	}

	virtual Ref< Movie > get() override final
	{
		return wait() ? m_movie : nullptr;
	}

private:
	std::wstring m_url;
	std::wstring m_cacheDirectory;
	bool m_merge;
	bool m_triangulate;
	Ref< Job > m_job;
	Ref< Movie > m_movie;

	void loader()
	{
		std::wstring cacheFileName = net::Url::encode(m_url);

		if (m_merge || m_triangulate)
		{
			cacheFileName += L"_";
			if (m_merge)
				cacheFileName += L"m";
			if (m_triangulate)
				cacheFileName += L"t";
		}

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmRead);
			if (f)
			{
				compress::InflateStreamLzf is(f);
				BufferedStream bs(&is);
				m_movie = BinarySerializer(&bs).readObject< Movie >();
				bs.close();
			}
			if (m_movie)
				return;
		}

		Ref< net::UrlConnection > connection = net::UrlConnection::open(net::Url(m_url));
		if (!connection)
			return;

		Ref< IStream > s = connection->getStream();
		T_ASSERT(s);

		std::wstring tempFile;
		Ref< IStream > d;

		for (int32_t i = 0; i < 10; ++i)
		{
			tempFile = OS::getInstance().getWritableFolderPath() + L"/" + cacheFileName + L"_" + toString(i);
			if ((d = FileSystem::getInstance().open(tempFile, File::FmWrite)) != nullptr)
				break;
		}
		if (!d)
			return;

		if (!StreamCopy(d, s).execute())
			return;

		d->close();
		s->close();

		d = FileSystem::getInstance().open(tempFile, File::FmRead);
		if (!d)
			return;

		std::wstring ext = toLower(traktor::Path(m_url).getExtension());

		// Try to load image and embedd into a movie first, if extension
		// not supported then this fail quickly.
		Ref< drawing::Image > image = drawing::Image::load(d, ext);
		if (image)
			m_movie = SwfMovieFactory().createMovieFromImage(image);
		else
		{
			SwfReader swfReader(d);
			m_movie = SwfMovieFactory().createMovie(&swfReader);
		}

		d->close();
		FileSystem::getInstance().remove(tempFile);

		if (!m_movie)
			return;

		if (m_merge)
			m_movie = Optimizer().merge(m_movie);

		if (m_triangulate)
			Optimizer().triangulate(m_movie, false);

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmWrite);
			if (f)
			{
				compress::DeflateStreamLzf ds(f);
				BufferedStream bs(&ds);
				BinarySerializer(&bs).writeObject(m_movie);
				bs.close();
			}
		}
	}
};

		}

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

Ref< IMovieLoader::IHandle > MovieLoader::loadAsync(const std::wstring& url) const
{
	return new MovieLoaderHandle(url, m_cacheDirectory, m_merge, m_triangulate);
}

Ref< Movie > MovieLoader::load(const std::wstring& url) const
{
	Ref< IHandle > handle = loadAsync(url);
	return handle ? handle->get() : nullptr;
}

	}
}
