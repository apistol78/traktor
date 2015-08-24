#include "Core/Date/DateTime.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpResponse.h"
#include "Telemetry/EventTask.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.EventTask", EventTask, ITask)

EventTask::EventTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol)
:	m_serverHost(serverHost)
,	m_client(client)
,	m_symbol(symbol)
,	m_timeStamp(DateTime::now().getSecondsSinceEpoch())
{
}

ITask::TaskResult EventTask::execute()
{
	StringOutputStream ss;
	ss << L"<?xml version=\"1.0\"?>";
	ss << L"<event>";
	ss <<	L"<client>" << m_client << L"</client>";
	ss <<	L"<symbol>" << m_symbol << L"</symbol>";
	ss <<	L"<timeStamp>" << m_timeStamp << L"</timeStamp>";
	ss << L"</event>";

	net::HttpClient client;
	Ref< net::HttpResponse > response = client.put(
		net::Url(L"http://" + m_serverHost + L"/Api/Event.php"),
		ss.str()
	);
	if (!response)
	{
		log::error << L"Unable to report action; no response from server." << Endl;
		return TrRetryAgainLater;
	}

	if (response->getStatusCode() != 200)
	{
		log::error << L"Unable to report action; error response from server \"" << response->getStatusMessage() << L"\" (" << response->getStatusCode() << L")." << Endl;
		return TrFailure;
	}

	return TrSuccess;
}

	}
}
