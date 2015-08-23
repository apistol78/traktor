#include "Core/Date/DateTime.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpResponse.h"
#include "Telemetry/AddValueTask.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.AddValueTask", AddValueTask, ITask)

AddValueTask::AddValueTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol, int32_t delta)
:	m_serverHost(serverHost)
,	m_client(client)
,	m_symbol(symbol)
,	m_delta(delta)
,	m_timeStamp(DateTime::now().getSecondsSinceEpoch())
{
}

ITask::TaskResult AddValueTask::execute()
{
	StringOutputStream ss;
	ss << L"<?xml version=\"1.0\"?>";
	ss << L"<addvalue>";
	ss <<	L"<client>" << m_client << L"</client>";
	ss <<	L"<symbol>" << m_symbol << L"</symbol>";
	ss <<	L"<delta>" << m_delta << L"</delta>";
	ss <<	L"<timeStamp>" << m_timeStamp << L"</timeStamp>";
	ss << L"</addvalue>";

	net::HttpClient client;
	Ref< net::HttpResponse > response = client.put(
		net::Url(L"http://" + m_serverHost + L"/Api/Add.php"),
		ss.str()
	);
	if (!response)
	{
		log::error << L"Unable to add value; no response from server." << Endl;
		return TrRetryAgainLater;
	}

	if (response->getStatusCode() != 200)
	{
		log::error << L"Unable to add value; error response from server \"" << response->getStatusMessage() << L"\" (" << response->getStatusCode() << L")." << Endl;
		return TrFailure;
	}

	return TrSuccess;
}

	}
}
