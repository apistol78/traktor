#ifndef traktor_OutputStream_H
#define traktor_OutputStream_H

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStreamBuffer;

/*! \brief Output stream.
 * \ingroup Core
 */
class T_DLLCLASS OutputStream : public Object
{
	T_RTTI_CLASS(OutputStream)

public:
	typedef OutputStream& (*manipulator_t)(OutputStream& s);

	OutputStream(OutputStreamBuffer* buffer = 0);

	virtual ~OutputStream();

	void setBuffer(OutputStreamBuffer* buffer);

	OutputStreamBuffer* getBuffer() const;

	OutputStream& operator << (manipulator_t m);

	OutputStream& operator << (const void* p);

	OutputStream& operator << (int8_t n);

	OutputStream& operator << (uint8_t n);

	OutputStream& operator << (int16_t n);

	OutputStream& operator << (uint16_t n);

	OutputStream& operator << (int32_t n);

	OutputStream& operator << (uint32_t n);

	OutputStream& operator << (int64_t n);

	OutputStream& operator << (uint64_t n);

	OutputStream& operator << (float f);

	OutputStream& operator << (double f);

	OutputStream& operator << (long double f);

	OutputStream& operator << (const wchar_t* s);

	OutputStream& operator << (const std::wstring& s);

	void put(wchar_t ch);

	void puts(const wchar_t* s);

	void flush();

	int getIndent() const;

	void setIndent(int indentCount);

	void increaseIndent();

	void decreaseIndent();

private:
	Semaphore m_lock;
	Ref< OutputStreamBuffer > m_buffer;
	std::vector< wchar_t > m_indent;
	bool m_pushIndent;
	std::vector< wchar_t > m_internal;
};

/*! \brief Character stream operator.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& operator << (OutputStream& os, wchar_t ch);

/*! \brief Scoped indentation.
 * \ingroup Core
 */
class T_DLLCLASS ScopeIndent
{
public:
	ScopeIndent(OutputStream& os)
	:	m_os(os)
	{
		m_indent = m_os.getIndent();
	}

	~ScopeIndent()
	{
		m_os.setIndent(m_indent);
	}

private:
	OutputStream& m_os;
	int m_indent;
};

/*! \brief End-of-line.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& Endl(OutputStream& s);

/*! \brief Increase indentation.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& IncreaseIndent(OutputStream& s);

/*! \brief Decrease indentation.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& DecreaseIndent(OutputStream& s);

/*! \brief Multiple lines with line numbers.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& FormatMultipleLines(OutputStream& s, const std::wstring& str);

}

#endif	// traktor_OutputStream_H
