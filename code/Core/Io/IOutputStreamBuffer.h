#ifndef traktor_IOutputStreamBuffer_H
#define traktor_IOutputStreamBuffer_H

#include "Core/Object.h"

namespace traktor
{

/*! \brief Buffer used by output streams.
 * \ingroup Core
 */
class IOutputStreamBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual int overflow(const wchar_t* buffer, int count) = 0;
};

}

#endif	// traktor_IOutputStreamBuffer_H
