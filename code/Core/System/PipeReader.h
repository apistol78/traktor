#ifndef traktor_PipeReader_H
#define traktor_PipeReader_H

#include <list>
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
	enum Result
	{
		RtOk,
		RtTimeout,
		RtEnd
	};

	PipeReader(IStream* stream);

	virtual ~PipeReader();

	Result readLine(std::wstring& outLine, int32_t timeout);

private:
	Ref< IStream > m_stream;
	std::vector< char > m_buffer;
	std::vector< char > m_acc;
	std::list< std::wstring > m_lines;
};

}

#endif	// traktor_PipeReader_H
