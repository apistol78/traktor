#ifndef traktor_PipeReader_H
#define traktor_PipeReader_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Process pipe line reader.
 * \ingroup Core
 */
class T_DLLCLASS PipeReader : public Object
{
	T_RTTI_CLASS;

public:
	PipeReader(IStream* stream);

	bool readLine(std::wstring& outLine, int32_t timeout);

private:
	Ref< IStream > m_stream;
	std::vector< char > m_acc;
};

}

#endif	// traktor_PipeReader_H
