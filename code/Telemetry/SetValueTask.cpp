/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Date/DateTime.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpRequestContent.h"
#include "Net/Http/HttpResponse.h"
#include "Telemetry/SetValueTask.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.SetValueTask", SetValueTask, ITask)

SetValueTask::SetValueTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol, int32_t value)
:	m_serverHost(serverHost)
,	m_client(client)
,	m_symbol(symbol)
,	m_value(value)
,	m_timeStamp(DateTime::now().getSecondsSinceEpoch())
{
}

ITask::TaskResult SetValueTask::execute()
{
	StringOutputStream ss;
	ss << L"<?xml version=\"1.0\"?>";
	ss << L"<setvalue>";
	ss <<	L"<client>" << m_client << L"</client>";
	ss <<	L"<symbol>" << m_symbol << L"</symbol>";
	ss <<	L"<value>" << m_value << L"</value>";
	ss <<	L"<timeStamp>" << m_timeStamp << L"</timeStamp>";
	ss << L"</setvalue>";

	net::HttpClient client;
	Ref< net::HttpResponse > response = client.put(
		net::Url(L"http://" + m_serverHost + L"/Api/Set.php"),
		net::HttpRequestContent(ss.str())
	);
	if (!response)
	{
		log::error << L"Unable to set value; no response from server." << Endl;
		return TrRetryAgainLater;
	}

	if (response->getStatusCode() != 200)
	{
		log::error << L"Unable to set value; error response from server \"" << response->getStatusMessage() << L"\" (" << response->getStatusCode() << L")." << Endl;
		return TrFailure;
	}

	return TrSuccess;
}

	}
}
