/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemoryStream_H
#define traktor_MemoryStream_H

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

/*! \brief Fixed memory stream.
 * \ingroup Core
 */
class T_DLLCLASS MemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	MemoryStream(void* buffer, int64_t bufferSize, bool readAllowed = true, bool writeAllowed = true, bool own = false);

	MemoryStream(const void* buffer, int64_t bufferSize);
	
	virtual ~MemoryStream();

	virtual void close() T_OVERRIDE;

	virtual bool canRead() const T_OVERRIDE;

	virtual bool canWrite() const T_OVERRIDE;

	virtual bool canSeek() const T_OVERRIDE;

	virtual int64_t tell() const T_OVERRIDE;

	virtual int64_t available() const T_OVERRIDE;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) T_OVERRIDE;

	virtual int64_t read(void* block, int64_t nbytes) T_OVERRIDE;

	virtual int64_t write(const void* block, int64_t nbytes) T_OVERRIDE;

	virtual void flush() T_OVERRIDE;

private:
	uint8_t* m_buffer;
	uint8_t* m_bufferPtr;
	int64_t m_bufferSize;
	bool m_readAllowed;
	bool m_writeAllowed;
	bool m_own;
};

}

#endif	// traktor_MemoryStream_H
