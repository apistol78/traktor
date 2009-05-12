#ifndef traktor_FileOutputStream_H
#define traktor_FileOutputStream_H

#include "Core/Io/OutputStream.h"

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
class Encoding;

/*! \brief File output stream.
 * \ingroup Core
 */
class T_DLLCLASS FileOutputStream : public OutputStream
{
	T_RTTI_CLASS(FileOutputStream)

public:
	FileOutputStream(Stream* stream, Encoding* encoding);

	void close();
}; 

}

#endif	// traktor_FileOutputStream_H
