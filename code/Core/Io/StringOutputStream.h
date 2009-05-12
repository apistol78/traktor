#ifndef traktor_StringOutputStream_H
#define traktor_StringOutputStream_H

#include <vector>
#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief StringOutputStream buffer.
 * \ingroup Core
 */
class StringOutputStreamBuffer : public OutputStreamBuffer
{
	T_RTTI_CLASS(StringOutputStreamBuffer)

protected:
	friend class StringOutputStream;
	
	std::vector< wchar_t > m_internal;

	virtual int overflow(const wchar_t* buffer, int count);
};

/*! \brief Formatting string stream.
 * \ingroup Core
 */
class T_DLLCLASS StringOutputStream : public OutputStream
{
	T_RTTI_CLASS(StringOutputStream)

public:
	StringOutputStream();

	virtual ~StringOutputStream();

	bool empty();

	std::wstring str();

	void reset();

private:
	StringOutputStreamBuffer m_buffer;
};

}

#endif	// traktor_StringOutputStream_H
