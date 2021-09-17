#include "Avalanche/Blob.h"
#include "Avalanche/Dictionary.h"
#include "Avalanche/Protocol.h"
#include "Avalanche/Server/Connection.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadPool.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Connection", Connection, Object)

Connection::Connection(Dictionary* dictionary)
:	m_dictionary(dictionary)
,	m_finished(false)
{
}

bool Connection::create(net::TcpSocket* clientSocket)
{
	m_clientSocket = clientSocket;

	auto fn = [=]()
	{
		log::info << L"Connection established, ready to process requests." << Endl;
		while (!m_thread->stopped())
		{
			if (!process())
				break;
		}
		log::info << L"Connection terminated." << Endl;
		m_finished = true;
	};

	if (!ThreadPool::getInstance().spawn(fn, m_thread))
		return false;

	return true;
}

bool Connection::update()
{
	return !m_finished;
}

bool Connection::process()
{
	net::SocketStream clientStream(m_clientSocket, true, true);

	int32_t cmd = m_clientSocket->recv();
	switch (cmd)
	{
	case c_commandPing:
		{
			if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
				return false;
		}
		break;

	case c_commandStat:
		{
			Key key = Key::read(&clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			Ref< const Blob > blob = m_dictionary->get(key);
			if (blob)
			{
				if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
					return false;

				int64_t blobSize = blob->size();
				if (clientStream.write(&blobSize, sizeof(blobSize)) != sizeof(blobSize))
					return false;
			}
			else
			{
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;
			}
		}
		break;

	case c_commandGet:
		{
			Key key = Key::read(&clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			Ref< const Blob > blob = m_dictionary->get(key);
			if (blob)
			{
				auto readStream = blob->read();
				if (readStream)
				{
					if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
						return false;

					int64_t blobSize = blob->size();
					if (clientStream.write(&blobSize, sizeof(blobSize)) != sizeof(blobSize))
						return false;

					if (!StreamCopy(&clientStream, readStream).execute(blob->size()))
					{
						log::error << L"[GET " << key.format() << L"] Unable to send " << blob->size() << L" byte(s) to client; terminating connection." << Endl;
						return false;
					}
					else
						log::info << L"[GET " << key.format() << L"] Sent " << blob->size() << L" bytes successfully." << Endl;
				}
				else
				{
					log::error <<  L"[GET " << key.format() << L"] Unable to acquire read stream from blob." << Endl;
					if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
						return false;
				}
			}
			else
			{
				log::info << L"[GET " << key.format() << L"] No such blob." << Endl;
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;
			}
		}
		break;

	case c_commandPut:
		{
			Key key = Key::read(&clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			if (m_dictionary->get(key) != nullptr)
			{
				log::error << L"[PUT " << key.format() << L"] Cannot replace already existing blob." << Endl;
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;

				return true;
			}

			Ref< Blob > blob = m_dictionary->create();
			if (blob)
			{
				if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
					return false;

				for (;;)
				{
					int32_t subcmd = m_clientSocket->recv();
					if (subcmd == c_subCommandPutAppend)
					{
						int64_t chunkSize;
						if (clientStream.read(&chunkSize, sizeof(chunkSize)) != sizeof(chunkSize))
							return false;

						auto appendStream = blob->append(chunkSize);
						if (appendStream)
						{
							if (StreamCopy(appendStream, &clientStream).execute(chunkSize))
							{
								if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
									return false;
							}
							else
							{
								log::error << L"[PUT " << key.format() << L"] Unable to receive " << chunkSize << L" byte(s) from client; terminating connection." << Endl;
								return false;
							}
						}
						else
						{
							log::error << L"[PUT " << key.format() << L"] Failed to append data to blob." << Endl;
							if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
								return false;
						}
					}
					else if (subcmd == c_subCommandPutCommit)
					{
						if (m_dictionary->put(key, blob, true))
						{
							log::info << L"[PUT " << key.format() << L"] Committed " << blob->size() << L" byte(s) to dictionary successfully." << Endl;
							if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
								return false;
						}
						else
						{
							if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
								return false;
						}
						break;
					}
					else if (subcmd == c_subCommandPutDiscard)
					{
						log::info << L"[PUT " << key.format() << L"] Discarded" << Endl;
						if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
							return false;
						break;
					}
					else
					{
						if (subcmd >= 0)
							log::error << L"[PUT " << key.format() << L"] Invalid sub-command from client; terminating connection." << Endl;
						return false;
					}
				}
			}
			else
			{
				log::error << L"[PUT " << key.format() << L"] Failed to create blob." << Endl;
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;
			}
		}
		break;

	case c_commandReplicate:
		{
			Key key = Key::read(&clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			if (m_dictionary->get(key) != nullptr)
			{
				log::error << L"[REP " << key.format() << L"] Cannot replace already existing blob." << Endl;
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;

				return true;
			}

			Ref< Blob > blob = m_dictionary->create();
			if (blob)
			{
				if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
					return false;

				for (;;)
				{
					int32_t subcmd = m_clientSocket->recv();
					if (subcmd == c_subCommandPutAppend)
					{
						int64_t chunkSize;
						if (clientStream.read(&chunkSize, sizeof(chunkSize)) != sizeof(chunkSize))
							return false;

						auto appendStream = blob->append(chunkSize);
						if (appendStream)
						{
							if (StreamCopy(appendStream, &clientStream).execute(chunkSize))
							{
								if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
									return false;
							}
							else
							{
								log::error << L"[REP " << key.format() << L"] Unable to receive " << chunkSize << L" byte(s) from client; terminating connection." << Endl;
								return false;
							}
						}
						else
						{
							log::error << L"[REP " << key.format() << L"] Failed to append data to blob." << Endl;
							if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
								return false;
						}
					}
					else if (subcmd == c_subCommandPutCommit)
					{
						if (m_dictionary->put(key, blob, false))
						{
							log::info << L"[REP " << key.format() << L"] Committed " << blob->size() << L" byte(s) to dictionary successfully." << Endl;
							if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
								return false;
						}
						else
						{
							if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
								return false;
						}
						break;
					}
					else if (subcmd == c_subCommandPutDiscard)
					{
						log::info << L"[REP " << key.format() << L"] Discarded" << Endl;
						if (clientStream.write(&c_replyOk, sizeof(c_replyOk)) != sizeof(c_replyOk))
							return false;
						break;
					}
					else
					{
						if (subcmd >= 0)
							log::error << L"[REP " << key.format() << L"] Invalid sub-command from client; terminating connection." << Endl;
						return false;
					}
				}
			}
			else
			{
				log::error << L"[REP " << key.format() << L"] Failed to create blob." << Endl;
				if (clientStream.write(&c_replyFailure, sizeof(c_replyFailure)) != sizeof(c_replyFailure))
					return false;
			}
		}
		break;

	default:
		if (cmd >= 0)
			log::error << L"Invalid command from client; terminating connection." << Endl;
		return false;
	}

	return true;
}

	}
}
