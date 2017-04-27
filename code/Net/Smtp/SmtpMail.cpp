/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/Smtp/SmtpMail.h"
#include "Net/SocketAddressIPv6.h"
#include "Net/TcpSocket.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

int waitReply(TcpSocket& socket)
{
	std::wstring reply = L"";
	char prev = 0;

	for (;;)
	{
		char ch;
		if (socket.recv(&ch, 1) != 1)
			return -1;

		reply += ch;
		if (prev == '\r' && ch == '\n')
			break;

		prev = ch;
	}

	return parseString< int >(reply);
}

int sendCommand(TcpSocket& socket, const std::wstring& cmd)
{
	std::wstring tmp = cmd + L"\r\n";
	if (socket.send(tmp.c_str(), int(tmp.length())) != int(tmp.length()))
		return -1;

	return waitReply(socket);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SmtpMail", SmtpMail, Object)

SmtpMail::SmtpMail(const std::wstring& server, int port) :
	m_server(server),
	m_port(port)
{
}

bool SmtpMail::send(const std::wstring& to, const std::wstring& from, const std::wstring& message)
{
	TcpSocket socket;

	if (!socket.connect(SocketAddressIPv6(m_server, m_port)))
		return false;

	if (waitReply(socket) != 220)
		return false;

	if (sendCommand(socket, L"HELO [127.0.0.1]") != 250)
		return false;

	if (sendCommand(socket, L"MAIL FROM:<" + from + L">") != 250)
		return false;

	if (sendCommand(socket, L"RCPT TO:<" + to + L">") != 250)
		return false;

	if (sendCommand(socket, L"DATA") != 354)
		return false;

	std::vector< std::wstring > data;
	Split< std::wstring >::any(message, L"\r\n", data);
	for (std::vector< std::wstring >::iterator i = data.begin(); i != data.end(); ++i)
	{
		if (socket.send(i->c_str(), int(i->length())) != int(i->length()))
			return false;
	}

	if (sendCommand(socket, L"\r\n.") != 250)
		return false;

	socket.close();
	return true;
}

	}
}
