#include <cstring>
#include <sstream>
#include "Editor/Pipeline/MemCachedGetStream.h"
#include "Editor/Pipeline/MemCachedProto.h"
#include "Core/Thread/Acquire.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedGetStream", MemCachedGetStream, IStream)

MemCachedGetStream::MemCachedGetStream(MemCachedProto* proto, const std::string& key)
:	m_proto(proto)
,	m_key(key)
,	m_inblock(0)
,	m_index(0)
{
}

bool MemCachedGetStream::requestEndBlock()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_proto->getLock());

	std::stringstream ss;
	std::string command;
	std::string reply;

	ss << "get " << m_key << ":END";
	
	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!m_proto->sendCommand(command))
	{
		log::error << L"Unable to request cache block; unable to send command" << Endl;
		return false;
	}

	bool gotEndBlock = false;
	for (;;)
	{
		if (!m_proto->readReply(reply))
		{
			log::error << L"Unable to request cache block; unable to receive reply" << Endl;
			return false;
		}

		std::vector< std::string > args;
		Split< std::string >::any(reply, " ", args);

		if (args.empty())
		{
			log::error << L"Unable to request cache block; empty reply" << Endl;
			return false;
		}

		if (args[0] == "VALUE")
		{
			if (args.size() < 4)
			{
				log::error << L"Unable to request cache block; malformed reply" << Endl;
				return false;
			}

			uint32_t bytes = parseString< uint32_t >(args[3]);
			if (bytes != 1)
			{
				log::error << L"Unable to request cache block; invalid size, end block must be one byte" << Endl;
				return false;
			}

			uint8_t endData = 0;
			if (!m_proto->readData(&endData, 1))
			{
				log::error << L"Unable to request cache block; unable to receive data" << Endl;
				return false;
			}
			
			if (endData != 0xfe)
			{
				log::error << L"Unable to request cache block; invalid end block" << Endl;
				return false;
			}
			
			gotEndBlock = true;
		}
		else
		{
			if (args[0] != "END")
				log::error << L"Unable to request cache block; server error " << mbstows(args[0]) << Endl;
			break;
		}
	}
	
	return gotEndBlock;
}

bool MemCachedGetStream::requestNextBlock()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_proto->getLock());

	std::stringstream ss;
	std::string command;
	std::string reply;

	ss << "get " << m_key << ":" << m_index;
	
	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!m_proto->sendCommand(command))
	{
		log::error << L"Unable to request cache block; unable to send command" << Endl;
		return false;
	}

	m_inblock = 0;

	for (;;)
	{
		if (!m_proto->readReply(reply))
		{
			log::error << L"Unable to request cache block; unable to receive reply" << Endl;
			return false;
		}

		std::vector< std::string > args;
		Split< std::string >::any(reply, " ", args);

		if (args.empty())
		{
			log::error << L"Unable to request cache block; empty reply" << Endl;
			return false;
		}

		if (args[0] == "VALUE")
		{
			if (args.size() < 4)
			{
				log::error << L"Unable to request cache block; malformed reply" << Endl;
				return false;
			}

			uint32_t bytes = parseString< uint32_t >(args[3]);
			T_DEBUG(L"\tbytes = " << bytes);

			uint32_t avail = MaxBlockSize - m_inblock;
			if (bytes > avail)
			{
				log::error << L"Unable to request cache block; data block too big" << Endl;
				return false;
			}

			if (!m_proto->readData(&m_block[m_inblock], bytes))
			{
				log::error << L"Unable to request cache block; unable to receive data" << Endl;
				return false;
			}

			m_inblock += bytes;
		}
		else
		{
			if (args[0] != "END")
				log::error << L"Unable to request cache block; server error " << mbstows(args[0]) << Endl;
			break;
		}
	}

	if (!m_inblock)
		return false;

	m_index++;
	return true;
}

void MemCachedGetStream::close()
{
	m_proto = 0;
}

bool MemCachedGetStream::canRead() const
{
	return true;
}

bool MemCachedGetStream::canWrite() const
{
	return false;
}

bool MemCachedGetStream::canSeek() const
{
	return false;
}

int64_t MemCachedGetStream::tell() const
{
	return 0;
}

int64_t MemCachedGetStream::available() const
{
	return 0;
}

int64_t MemCachedGetStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t MemCachedGetStream::read(void* block, int64_t nbytes)
{
	uint8_t* writePtr = static_cast< uint8_t* >(block);
	int64_t navail = nbytes;

	while (navail > 0)
	{
		if (m_inblock)
		{
			int32_t nget = std::min< int32_t >(navail, m_inblock);

			std::memcpy(writePtr, m_block, nget);
			std::memmove(m_block, &m_block[nget], m_inblock - nget);

			m_inblock -= nget;
			navail -= nget;
			writePtr += nget;
		}
		else
		{
			if (!requestNextBlock())
				break;
		}
	}

	return nbytes - navail;
}

int64_t MemCachedGetStream::write(const void* block, int64_t nbytes)
{
	return 0;
}

void MemCachedGetStream::flush()
{
}

	}
}
