#pragma once

#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"
#include "Core/Misc/AutoPtr.h"

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

	const wchar_t* c_str() const;

	void reset();

	virtual int32_t overflow(const wchar_t* buffer, int32_t count);

private:
	friend class StringOutputStream;

	AutoArrayPtr< wchar_t, AllocatorFree > m_buffer;
	size_t m_capacity;
	size_t m_tail;
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

	const wchar_t* c_str() const;

	void reset();

private:
	StringOutputStreamBuffer m_buffer;
};

}
