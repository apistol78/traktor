#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Amalgam/Impl/TargetID.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetManager", TargetManager, Object)

bool TargetManager::create(uint16_t port)
{
	// Create our server socket.
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(port)))
		return false;

	if (!m_listenSocket->listen())
		return false;

	return true;
}

void TargetManager::destroy()
{
	// Destroy instances.
	for (RefArray< TargetInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->destroy();

	m_instances.clear();

	// Close server socket.
	if (m_listenSocket)
	{
		m_listenSocket->close();
		m_listenSocket = 0;
	}
}

TargetInstance* TargetManager::createInstance(const std::wstring& name, const Target* target)
{
	// Create target instance; add to our list of instances.
	Ref< TargetInstance > instance = new TargetInstance(name, target);
	m_instances.push_back(instance);
	return instance;
}

void TargetManager::update()
{
	// Check if any pending connection available.
	if (m_listenSocket->select(true, false, false, 0))
	{
		// Accept new connection.
		Ref< net::TcpSocket > socket = m_listenSocket->accept();
		if (socket)
		{
			// Target must send it's identifier upon connection.
			Ref< TargetID > targetId = BinarySerializer(&net::SocketStream(socket, true, false)).readObject< TargetID >();
			if (targetId)
			{
				// Find instance with matching identifier.
				TargetInstance* instance = 0;
				for (RefArray< TargetInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
				{
					if ((*i)->getId() == targetId->getId())
					{
						instance = *i;
						break;
					}
				}

				if (instance)
				{
					// Create connection object and add to instance.
					instance->addConnection(new TargetConnection(socket));
					log::info << L"New target connection accepted; ID " << targetId->getId().format() << Endl;
				}
				else
				{
					// Unknown target; refusing connection.
					socket->close();
					log::error << L"Unknown target; connection refused" << Endl;
				}
			}
			else
			{
				// Invalid target ID message; refusing connection.
				socket->close();
				log::error << L"Invalid target ID message; connection refused" << Endl;
			}
		}
	}

	// Update all instances.
	for (RefArray< TargetInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->update();
}

	}
}
