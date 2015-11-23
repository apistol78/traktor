#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Functor/Functor.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Semaphore.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/SwfReader.h"
#include "Net/UrlConnection.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class FlashMovieLoaderHandle : public IFlashMovieLoader::IHandle
{
public:
	FlashMovieLoaderHandle(const net::Url& url, const std::wstring& cacheDirectory, bool merge)
	:	m_url(url)
	,	m_cacheDirectory(cacheDirectory)
	,	m_merge(merge)
	{
		m_job = JobManager::getInstance().add(makeFunctor< FlashMovieLoaderHandle >(this, &FlashMovieLoaderHandle::loader));
	}

	virtual bool wait() T_OVERRIDE T_FINAL
	{
		return m_job ? m_job->wait() : true;
	}

	virtual bool ready() T_OVERRIDE T_FINAL
	{
		return m_job ? m_job->stopped() : true;
	}

	virtual bool succeeded() T_OVERRIDE T_FINAL
	{
		return wait() ? (m_movie != 0) : false;
	}

	virtual Ref< FlashMovie > get() T_OVERRIDE T_FINAL
	{
		return wait() ? m_movie : 0;
	}

private:
	net::Url m_url;
	std::wstring m_cacheDirectory;
	bool m_merge;
	Ref< Job > m_job;
	Ref< FlashMovie > m_movie;
#if defined(__ANDROID__) || defined(__IOS__)
	static Semaphore ms_lock;
#endif

	void loader()
	{
		std::wstring cacheFileName = net::Url::encode(m_url.getString());

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmRead);
			if (f)
			{
				compress::InflateStreamLzf is(f);
				m_movie = BinarySerializer(&is).readObject< FlashMovie >();
				is.close();
			}
			if (m_movie)
				return;
		}

		Ref< net::UrlConnection > connection = net::UrlConnection::open(m_url);
		if (!connection)
			return;

		Ref< IStream > s = connection->getStream();
		T_ASSERT (s);

		std::wstring tempFile = OS::getInstance().getWritableFolderPath() + L"/" + cacheFileName;

		Ref< IStream > d = FileSystem::getInstance().open(tempFile, File::FmWrite);
		if (!d)
			return;

		if (!StreamCopy(d, s).execute())
			return;

		d->close();
		s->close();

		d = FileSystem::getInstance().open(tempFile, File::FmRead);
		if (!d)
			return;

#if defined(__ANDROID__) || defined(__IOS__)
		ms_lock.wait();
#endif
		SwfReader swfReader(d);
		m_movie = FlashMovieFactory().createMovie(&swfReader);

#if defined(__ANDROID__) || defined(__IOS__)
		ms_lock.release();
#endif
		d->close();

		FileSystem::getInstance().remove(tempFile);

		if (!m_movie)
			return;

		if (m_merge)
			m_movie = FlashOptimizer().merge(m_movie);

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmWrite);
			if (f)
			{
				compress::DeflateStreamLzf ds(f);
				BinarySerializer(&ds).writeObject(m_movie);
				ds.close();
			}
		}
	}
};

#if defined(__ANDROID__) || defined(__IOS__)
Semaphore FlashMovieLoaderHandle::ms_lock;
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieLoader", FlashMovieLoader, IFlashMovieLoader)

FlashMovieLoader::FlashMovieLoader()
:	m_merge(false)
{
}

void FlashMovieLoader::setCacheDirectory(const std::wstring& cacheDirectory)
{
	m_cacheDirectory = cacheDirectory;
}

void FlashMovieLoader::setMerge(bool merge)
{
	m_merge = merge;
}

Ref< IFlashMovieLoader::IHandle > FlashMovieLoader::loadAsync(const net::Url& url) const
{
	return new FlashMovieLoaderHandle(url, m_cacheDirectory, m_merge);
}

Ref< FlashMovie > FlashMovieLoader::load(const net::Url& url) const
{
	Ref< FlashMovie > movie;

	if (!m_cacheDirectory.empty())
	{
		std::wstring cacheFileName = net::Url::encode(url.getString());
		Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmRead);
		if (f)
		{
			compress::InflateStreamLzf is(f);
			movie = BinarySerializer(&is).readObject< FlashMovie >();
			is.close();
		}
		if (movie)
			return movie;
	}

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection)
		return 0;

	Ref< IStream > s = connection->getStream();
	T_ASSERT (s);

	DynamicMemoryStream dms;
	if (!StreamCopy(&dms, s).execute())
		return 0;

	dms.seek(IStream::SeekSet, 0);

	SwfReader swfReader(&dms);
	movie = FlashMovieFactory().createMovie(&swfReader);
	if (!movie)
		return 0;

	if (m_merge)
		movie = FlashOptimizer().merge(movie);

	if (!m_cacheDirectory.empty())
	{
		std::wstring cacheFileName = net::Url::encode(url.getString());
		Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmWrite);
		if (f)
		{
			compress::DeflateStreamLzf ds(f);
			BinarySerializer(&ds).writeObject(movie);
			ds.close();
		}
	}

	return movie;
}

	}
}
