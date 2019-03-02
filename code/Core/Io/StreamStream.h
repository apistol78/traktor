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

/*! \brief Stream in stream.
 * \ingroup Core
 */
class T_DLLCLASS StreamStream : public IStream
{
	T_RTTI_CLASS;

public:
	StreamStream(IStream* stream, int64_t endOffset = -1);

	virtual void close() override;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

protected:
	Ref< IStream > m_stream;
	int64_t m_startOffset;
	int64_t m_endOffset;
};

}

