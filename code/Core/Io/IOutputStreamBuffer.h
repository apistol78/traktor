#ifndef traktor_IOutputStreamBuffer_H
#define traktor_IOutputStreamBuffer_H

#include "Core/Object.h"

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
class T_DLLCLASS IOutputStreamBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual int overflow(const wchar_t* buffer, int count) = 0;
};

}

#endif	// traktor_IOutputStreamBuffer_H
