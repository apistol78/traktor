#ifndef traktor_OutputStreamBuffer_H
#define traktor_OutputStreamBuffer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Buffer used by output streams.
 * \ingroup Core
 */
class T_DLLCLASS OutputStreamBuffer : public Object
{
	T_RTTI_CLASS(OutputStreamBuffer)

protected:
	friend class OutputStream;

	virtual int overflow(const wchar_t* buffer, int count) = 0;
};

}

#endif	// traktor_OutputStreamBuffer_H
