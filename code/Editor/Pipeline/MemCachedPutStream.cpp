#include <algorithm>
#include <cstring>
#include <sstream>
#include "Editor/Pipeline/MemCachedPutStream.h"
#include "Editor/Pipeline/MemCachedProto.h"
#include "Core/Thread/Acquire.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPutStream", MemCachedPutStream, IStream)

MemCachedPutStream::MemCachedPutStream(MemCachedProto* proto, const std::string& key)
:	m_proto(proto)
,	m_key(key)
,	m_inblock(0)
,	m_index(0)
{
}

void MemCachedPutStream::close()
{
	if (m_proto)
	{
		flush();
		uploadEndBlock();
		m_proto = 0;
	}
}

bool MemCachedPutStream::canRead() const
{
	return false;
}

bool MemCachedPutStream::canWrite() const
{
	return true;
}

bool MemCachedPutStream::canSeek() const
{
	return false;
}

int64_t MemCachedPutStream::tell() const
{
	return 0;
}

int64_t MemCachedPutStream::available() const
{
	return 0;
}

int64_t MemCachedPutStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t MemCachedPutStream::read(void* block, int64_t nbytes)
{
	return 0;
}

int64_t MemCachedPutStream::write(const void* block, int64_t nbytes)
{
	const uint8_t* blockPtr = static_cast< const uint8_t* >(block);
	int64_t written = 0;

	while (written < nbytes)
	{
		int64_t avail = MaxBlockSize - m_inblock;
		int64_t copy = std::min(nbytes - written, avail);

		std::memcpy(&m_block[m_inblock], blockPtr, copy);

		blockPtr += copy;
		m_inblock += copy;
		written += copy;

		if (m_inblock >= MaxBlockSize)
		{
			if (uploadBlock())
				m_inblock = 0;
			else
				break;
		}
	}

	return written;
}

void MemCachedPutStream::flush()
{
	if (m_inblock > 0)
		uploadBlock();
}

bool MemCachedPutStream::uploadBlock()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_proto->getLock());

	std::stringstream ss;
	std::string command;
	std::string reply;

	ss << "set " << m_key << ":" << m_index << " 0 0 " << m_inblock;

	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!m_proto->sendCommand(command))
	{
		log::error << L"Unable to store cache block; unable to send command" << Endl;
		return false;
	}

	if (!m_proto->writeData(m_block, m_inblock))
	{
		log::error << L"Unable to store cache block; unable to write data" << Endl;
		return false;
	}

	if (!m_proto->readReply(reply))
	{
		log::error << L"Unable to store cache block; unable to read reply" << Endl;
		return false;
	}

	if (reply != "STORED")
	{
		log::error << L"Unable to store cache block; server unable to store data" << Endl;
		return false;
	}

	m_index++;
	return true;
}

void MemCachedPutStream::uploadEndBlock()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_proto->getLock());

	std::stringstream ss;
	std::string command;
	std::string reply;
	
	ss << "set " << m_key << ":END 0 0 1";

	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!m_proto->sendCommand(command))
	{
		log::error << L"Unable to store cache block; unable to send command" << Endl;
		return;
	}
	
	uint8_t endData = 0xfe;
	if (!m_proto->writeData(&endData, 1))
	{
		log::error << L"Unable to store cache block; unable to write data" << Endl;
		return;
	}

	if (!m_proto->readReply(reply))
	{
		log::error << L"Unable to store cache block; unable to read reply" << Endl;
		return;
	}

	if (reply != "STORED")
	{
		log::error << L"Unable to store cache block; server unable to store data" << Endl;
		return;
	}
}

	}
}
