#include "Core/Thread/Job.h"
#include "Net/Http/HttpClientResult.h"

namespace traktor
{
	namespace net
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpClientResult", HttpClientResult, Result)

void HttpClientResult::succeed(const HttpResponse* response, IStream* stream)
{
	m_response = response;
	m_stream = stream;
	Result::succeed();
}

const HttpResponse* HttpClientResult::getResponse()
{
	if (succeeded())
		return m_response;
	else
		return 0;
}

IStream* HttpClientResult::getStream()
{
	if (succeeded())
		return m_stream;
	else
		return 0;
}

	}
}
