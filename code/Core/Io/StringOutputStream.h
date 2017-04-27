/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StringOutputStream_H
#define traktor_StringOutputStream_H

#include <vector>
#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief StringOutputStream buffer.
 * \ingroup Core
 */
class StringOutputStreamBuffer : public OutputStreamBuffer
{
public:
	StringOutputStreamBuffer();

	bool empty() const;

	std::wstring str() const;

	void reset();

	virtual int32_t overflow(const wchar_t* buffer, int32_t count);

private:
	friend class StringOutputStream;

	std::vector< wchar_t > m_internal;
};

/*! \brief Formatting string stream.
 * \ingroup Core
 */
class T_DLLCLASS StringOutputStream : public OutputStream
{
	T_RTTI_CLASS;

public:
	StringOutputStream();

	virtual ~StringOutputStream();

	bool empty() const;

	std::wstring str() const;

	void reset();

private:
	StringOutputStreamBuffer m_buffer;
};

}

#endif	// traktor_StringOutputStream_H
