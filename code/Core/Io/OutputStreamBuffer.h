#ifndef traktor_OutputStreamBuffer_H
#define traktor_OutputStreamBuffer_H

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

/*! \brief Buffer used by output streams.
 * \ingroup Core
 */
class T_DLLCLASS OutputStreamBuffer : public IOutputStreamBuffer
{
	T_RTTI_CLASS;

public:
	OutputStreamBuffer();

	virtual int32_t getIndent() const;

	virtual void setIndent(int32_t indent);

	virtual int32_t getDecimals() const;

	virtual void setDecimals(int32_t decimals);

	virtual bool getPushIndent() const;

	virtual void setPushIndent(bool pushIndent);

	virtual int32_t overflow(const wchar_t* buffer, int32_t count) = 0;

private:
	int32_t m_indent;
	int32_t m_decimals;
	bool m_pushIndent;
};

}

#endif	// traktor_OutputStreamBuffer_H
