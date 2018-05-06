/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StreamStream_H
#define traktor_StreamStream_H

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

	virtual void close() T_OVERRIDE;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int64_t tell() const T_OVERRIDE T_FINAL;

	virtual int64_t available() const T_OVERRIDE T_FINAL;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) T_OVERRIDE T_FINAL;

	virtual int64_t read(void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual int64_t write(const void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

protected:
	Ref< IStream > m_stream;
	int64_t m_startOffset;
	int64_t m_endOffset;
};

}

#endif	// traktor_StreamStream_H
