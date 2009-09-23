#include "Core/Io/AsyncStream.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.AsyncStream", AsyncStream, Stream)

AsyncStream::AsyncStream(Stream* source)
:	m_bytesInChunks(0)
,	m_source(source)
,	m_maxChunkSize(64 * 1024)
,	m_maxChunkCount(32)
{
	T_ASSERT (m_source);
	T_ASSERT (m_source->canRead());

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &AsyncStream::threadRead), L"AsyncStream thread");
	m_thread->start(Thread::Normal);
}

AsyncStream::~AsyncStream()
{
	T_EXCEPTION_GUARD_BEGIN
	
	close();

	T_EXCEPTION_GUARD_END
}

void AsyncStream::close()
{
	if (m_thread)
	{
		m_thread->wait();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = 0;
	}
	if (m_source)
	{	
		m_source->close();
		m_source = 0;
	}
}

bool AsyncStream::canRead() const
{
	return bool(m_source != 0);
}

bool AsyncStream::canWrite() const
{
	return false;
}

bool AsyncStream::canSeek() const
{
	return m_source->canSeek();
}

int AsyncStream::tell() const
{
	Acquire< Mutex > lock(m_chunksLock);
	return m_source->tell() - m_bytesInChunks;
}

int AsyncStream::available() const
{
	if (!m_thread)
		return 0;

	Acquire< Mutex > lock(m_chunksLock);
	
	int avail = m_bytesInChunks;
	if (!m_thread->finished())
		avail += m_source->available();
	
	return avail;
}

int AsyncStream::seek(SeekOriginType origin, int offset)
{
	Acquire< Mutex > lock(m_chunksLock);

	switch (origin)
	{
	case SeekCurrent:
		if (offset > 0)
		{
			if (offset <= m_bytesInChunks)
			{
				// Drop data from chunks.
				while (offset > 0)
				{
					Chunk& front = m_chunks.front();
					if (offset > m_chunks.front().first)
					{
						// Offset span past this chunk, drop it completely.
						offset -= front.first;
						m_bytesInChunks -= front.first;
						delete[] front.second;
						m_chunks.pop_front();
					}
					else
					{
						// Enough data in current chunk to complete seek.
						memmove(front.second, &front.second[offset], front.first - offset);
						front.first -= offset;
						m_bytesInChunks -= offset;
						break;
					}
				}
			}
			else
			{
				// Span all chunks, drop them all and seek in source stream.
			}
		}
		else
		{
			// Negative seek, drop all chunks and seek in source stream.
		}
		break;
		
	case SeekEnd:
		break;
		
	case SeekSet:
		// Undeterministic seek, drop all chunks and seek in source stream.
		for (std::list< Chunk >::iterator i = m_chunks.begin(); i != m_chunks.end(); ++i)
			delete[] i->second;
			
		m_chunks.clear();
		m_bytesInChunks = 0;
		
		m_source->seek(SeekSet, offset);
		break;
	}

	return 0;
}

int AsyncStream::read(void* block, int nbytes)
{
	char* data = static_cast< char* >(block);
	int original = nbytes;
	
	while (nbytes > 0)
	{
		if (!m_chunks.empty())
		{
			// Data is available, pop chunks of queue.
			while (nbytes > 0)
			{
				m_chunksLock.acquire();
				if (nbytes >= m_chunks.front().first)
				{
					// Pop chunk from list as we have to consume the entire chunk.
					Chunk chunk = m_chunks.front();
					m_chunks.pop_front();
					
					// Decrement counter here as we want to be safe in case of thread switch.
					m_bytesInChunks -= chunk.first;
					
					// Now safe to unlock chunks again as we will operate on our own chunk copy.
					m_chunksLock.release();
					
					// Signal that we've removed a chunk from the queue, thread might be waiting for more space.
					m_chunksPopEvent.pulse();
					
					// Copy chunk data into output buffer.
					memcpy(data, chunk.second, chunk.first);
					delete[] chunk.second;
					
					// Update pointers and counters.
					data += chunk.first;
					nbytes -= chunk.first;
				}
				else
				{
					// Extract part of chunk only.
					Chunk& chunk = m_chunks.front();
					
					// Copy and move chunk data.
					memcpy(data, chunk.second, nbytes);
					memmove(chunk.second, &chunk.second[nbytes], chunk.first - nbytes);
					
					// Update pointers and counters.
					m_bytesInChunks -= nbytes;
					chunk.first -= nbytes;
					data += nbytes;
					nbytes = 0;
					
					// Unlock chunks.
					m_chunksLock.release();
				}
			}
		}
		else
		{
			// No data is available, wait until reader thread has received data.
			// This means user is actually reading too fast resulting in no performance
			// gain using this class.
			if (m_thread && !m_thread->finished())
			{
				log::warning << L"Chunk queue starved, stalled!" << Endl;
				m_chunksPushEvent.wait();
			}
			else
			{
				// Reader thread has terminated, cannot expect more data.
				break;
			}
		}
	}
	
	return original - nbytes;
}

int AsyncStream::write(const void* block, int nbytes)
{
	return 0;
}

void AsyncStream::flush()
{
}

void AsyncStream::threadRead()
{
	size_t chunksSize = 0;
	for (;;)
	{
		// Wait until chunks are read, we do not want to exceed maximum number of chunks.
		// This means user read too slow, resulting in full buffers. Try increase
		// buffer size to increase performance.
		while (chunksSize >= m_maxChunkCount)
		{
			log::warning << L"Chunk queue is full, reader thread stalled!" << Endl;
			if (!m_chunksPopEvent.wait())
				continue;
				
			m_chunksLock.acquire();
			chunksSize = m_chunks.size();
			m_chunksLock.release();
		}
	
		// Check if data is available from source stream.
		int avail = m_source->available();
		if (avail <= 0)
			break;
		
		// Determine how much data we should read, do not exceed maximum chunk size.
		size_t read = std::min< size_t >(avail, m_maxChunkSize);
		char* data = new char [read];
		
		// Read chunk from source stream.
		int got = m_source->read(data, int(read));
		if (got <= 0)
		{
			delete[] data;
			continue;
		}
		
		// Add chunk to buffer, set event to indicate new data is available.
		m_chunksLock.acquire();
		m_chunks.push_back(Chunk(got, data));
		m_bytesInChunks += got;
		chunksSize = m_chunks.size();
		m_chunksLock.release();
		m_chunksPushEvent.pulse();
	}
}

}
