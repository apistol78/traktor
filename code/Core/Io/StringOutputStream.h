#ifndef traktor_StringOutputStream_H
#define traktor_StringOutputStream_H

#include <vector>
#include "Core/Io/OutputStream.h"
#include "Core/Io/IOutputStreamBuffer.h"

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
class StringOutputStreamBuffer : public IOutputStreamBuffer
{
public:
	virtual int overflow(const wchar_t* buffer, int count);

private:
	friend class StringOutputStream;

	std::vector< wchar_t > m_internal;
};

/*! \brief Formatting string stream.
 * \ingroup Core
 */
class T_DLLCLASS StringOutputStream : public OutputStream
{
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
