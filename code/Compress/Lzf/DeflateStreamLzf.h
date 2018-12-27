/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_compress_DeflateStreamLzf_H
#define traktor_compress_DeflateStreamLzf_H

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace compress
	{

class DeflateLzfImpl;

/*! \brief LZF deflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS DeflateStreamLzf : public IStream
{
	T_RTTI_CLASS;

public:
	DeflateStreamLzf(IStream* stream, uint32_t blockSize = 64 * 1024);

	virtual ~DeflateStreamLzf();

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
	Ref< DeflateLzfImpl > m_impl;
};

	}
}

#endif	// traktor_compress_DeflateStreamLzf_H
