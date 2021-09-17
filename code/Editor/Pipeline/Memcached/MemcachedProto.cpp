#include <cstring>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Editor/Pipeline/Memcached/MemcachedProto.h"
#include "Net/Socket.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemcachedProto", MemcachedProto, Object)

MemcachedProto::MemcachedProto(net::Socket* socket)
:	m_socket(socket)
{
}

bool MemcachedProto::sendCommand(const std::string& command)
{
	uint32_t length = command.length();
	uint8_t buf[1024];

	std::memcpy(buf, command.c_str(), length);
	std::memcpy(&buf[length], "\r\n", 2);

	if (m_socket->send(buf, length + 2) != length + 2)
		return false;

	return true;
}

bool MemcachedProto::readReply(std::string& outReply)
{
	StaticVector< char, 1024 > buffer;
	for (;;)
	{
		if (m_socket->select(true, false, false, 1000) <= 0)
		{
			log::error << L"Unable to request cache block; timeout while waiting on reply." << Endl;
			return false;
		}

		char ch;
		if (m_socket->recv(&ch, sizeof(ch)) != sizeof(ch))
		{
			log::error << L"Unable to request cache block; unable to read reply." << Endl;
			return false;
		}

		buffer.push_back(ch);

		size_t i = buffer.size();
		if (i >= 2 && buffer[i - 2] == '\r' && buffer[i - 1] == '\n')
		{
			outReply = std::string(buffer.begin(), buffer.end() - 2);
			break;
		}
	}
	return true;
}

bool MemcachedProto::readData(uint8_t* data, uint32_t dataLength)
{
	int32_t dataReceived = 0;
	while (dataReceived < dataLength + 2)
	{
		if (m_socket->select(true, false, false, 1000) <= 0)
			return false;

		int32_t nbytes = dataLength + 2 - dataReceived;
		int32_t result = m_socket->recv(&data[dataReceived], nbytes);
		if (result < 0)
			return false;

		dataReceived += result;
	}

	const uint8_t* eod = &data[dataLength];
	return eod[0] == '\r' && eod[1] == '\n';
}

bool MemcachedProto::writeData(uint8_t* data, uint32_t dataLength)
{
	data[dataLength] = '\r';
	data[dataLength + 1] = '\n';

	if (m_socket->send(data, dataLength + 2) != dataLength + 2)
		return false;

	return true;
}

	}
}
