#include "Core/Functor/Functor.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
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
	FlashMovieLoaderHandle(const net::Url& url, bool merge)
	:	m_url(url)
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
	bool m_merge;
	Ref< Job > m_job;
	Ref< FlashMovie > m_movie;

	void loader()
	{
		Ref< net::UrlConnection > connection = net::UrlConnection::open(m_url);
		if (!connection)
			return;

		Ref< IStream > s = connection->getStream();
		T_ASSERT (s);

		DynamicMemoryStream dms;
		if (!StreamCopy(&dms, s).execute())
			return;

		dms.seek(IStream::SeekSet, 0);

		SwfReader swfReader(&dms);
		m_movie = FlashMovieFactory().createMovie(&swfReader);
		if (!m_movie)
			return;

		if (m_merge)
			m_movie = FlashOptimizer().merge(m_movie);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieLoader", FlashMovieLoader, IFlashMovieLoader)

FlashMovieLoader::FlashMovieLoader(bool merge)
:	m_merge(merge)
{
}

Ref< IFlashMovieLoader::IHandle > FlashMovieLoader::loadAsync(const net::Url& url) const
{
	return new FlashMovieLoaderHandle(url, m_merge);
}

Ref< FlashMovie > FlashMovieLoader::load(const net::Url& url) const
{
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
	Ref< FlashMovie > movie = FlashMovieFactory().createMovie(&swfReader);
	if (!movie)
		return 0;

	return m_merge ? FlashOptimizer().merge(movie) : movie;
}

	}
}
