#ifndef traktor_StreamCopy_H
#define traktor_StreamCopy_H

#include "Core/Heap/Ref.h"
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

class Stream;

/*! \brief Stream copy.
 * \ingroup Core
 *
 * Copy entire stream data from one stream
 * to another.
 */
class T_DLLCLASS StreamCopy : public Object
{
	T_RTTI_CLASS(StreamCopy)

public:
	StreamCopy(Stream* target, Stream* source);

	bool execute();

private:
	Ref< Stream > m_target;
	Ref< Stream > m_source;
};

}

#endif	// traktor_StreamCopy_H
