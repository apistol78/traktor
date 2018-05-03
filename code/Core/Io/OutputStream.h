/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_OutputStream_H
#define traktor_OutputStream_H

#include <string>
#include <vector>
#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Output stream.
 * \ingroup Core
 */
class T_DLLCLASS OutputStream : public Object
{
	T_RTTI_CLASS;

public:
	enum LineEnd
	{
		LeAuto = 0,
		LeWin = 1,
		LeMac = 2,
		LeUnix = 3
	};

	typedef OutputStream& (*manipulator_t)(OutputStream& s);

	OutputStream(IOutputStreamBuffer* buffer = 0, LineEnd lineEnd = LeAuto);

	void setBuffer(IOutputStreamBuffer* buffer);

	IOutputStreamBuffer* getBuffer() const;

	void setLineEnd(LineEnd lineEnd);

	LineEnd getLineEnd() const;

	OutputStream& operator << (manipulator_t m);

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

#if defined(__APPLE__)
	OutputStream& operator << (size_t n) { return *this << (uint32_t)n; }
#endif

	void put(wchar_t ch);

	void puts(const wchar_t* s);

	int32_t getIndent() const;

	void setIndent(int32_t indent);

	void increaseIndent();

	void decreaseIndent();

	int32_t getDecimals() const;

	void setDecimals(int32_t decimals);
	
	bool isEol(wchar_t ch) const;

private:
	Ref< IOutputStreamBuffer > m_buffer;
	LineEnd m_lineEnd;
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

/*! \brief Format as hexadecimal number.
 * \ingroup Core
 */
T_DLLCLASS OutputStream& FormatHex(OutputStream& s, uint32_t number, uint8_t nibbles);

}

#endif	// traktor_OutputStream_H
