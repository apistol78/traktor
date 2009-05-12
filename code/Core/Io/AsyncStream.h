#ifndef traktor_AsyncStream_H
#define traktor_AsyncStream_H

#include <utility>
#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Event.h"
#include "Core/Io/Stream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

/*! \brief Asynchronous stream.
 * \ingroup Core
 */
class T_DLLCLASS AsyncStream : public Stream
{
	T_RTTI_CLASS(AsyncStream)

public:
	AsyncStream(Stream* source);
	
	virtual ~AsyncStream();
	
	virtual void close();

	virtual bool canRead() const;

	virtual bool canWrite() const;

	virtual bool canSeek() const;

	virtual int tell() const;

	virtual int available() const;

	virtual int seek(SeekOriginType origin, int offset);

	virtual int read(void* block, int nbytes);

	virtual int write(const void* block, int nbytes);

	virtual void flush();
	
private:
	typedef std::pair< int, char* > Chunk;
	
	std::list< Chunk > m_chunks;
	int m_bytesInChunks;
	Ref< Stream > m_source;
	Thread* m_thread;
	mutable Mutex m_chunksLock;
	mutable Event m_chunksPushEvent;
	mutable Event m_chunksPopEvent;
	
	size_t m_maxChunkSize;
	size_t m_maxChunkCount;
	
	void threadRead();
};
	
}

#endif	// traktor_AsyncStream_H
