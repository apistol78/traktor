#ifndef traktor_amalgam_PipeReader_H
#define traktor_amalgam_PipeReader_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace amalgam
	{

class PipeReader : public Object
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
}

#endif	// traktor_amalgam_PipeReader_H
