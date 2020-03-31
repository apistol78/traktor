#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ChunkMemory;

/*! Chunk memory stream.
 * \ingroup Core
 * 
 * This stream maintain blocks of data, chunks,
 * which can be read or written contineously.
 * 
 * Useful for large in-memory data streams
 * where dynamic memory stream cannot be used due
 * to significant cost of reallocations.
 */
class T_DLLCLASS ChunkMemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	ChunkMemoryStream(ChunkMemory* memory, bool readAllowed = true, bool writeAllowed = true);

	virtual ~ChunkMemoryStream();

	virtual void close() override final;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

private:
	Ref< ChunkMemory > m_memory;
	int64_t m_position;
	bool m_readAllowed;
	bool m_writeAllowed;
};

}
