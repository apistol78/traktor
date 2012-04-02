#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Amalgam/Impl/TargetID.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
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
	m_instances.clear();

	// Close server socket.
	if (m_listenSocket)
	{
		m_listenSocket->close();
		m_listenSocket = 0;
	}
}

void TargetManager::addInstance(TargetInstance* targetInstance)
{
	m_instances.push_back(targetInstance);
}

void TargetManager::removeInstance(TargetInstance* targetInstance)
{
	m_instances.remove(targetInstance);
}

bool TargetManager::update()
{
	net::SocketSet socketSet, socketSetResult;
	bool needUpdate = false;

	// Update all targets; if any has been disconnected then we return true in order
	// to update user interface.
	for (RefArray< TargetInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		if ((*i)->update())
			needUpdate |= true;
	}

	// Gather all sockets so we can wait on all simultaneously.
	socketSet.add(m_listenSocket);
	for (RefArray< TargetInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		const RefArray< TargetConnection>& connections = (*i)->getConnections();
		for (RefArray< TargetConnection >::const_iterator j = connections.begin(); j != connections.end(); ++j)
			socketSet.add((*j)->getSocket());
	}

	// Wait on all sockets.
	if (socketSet.select(true, false, false, 100, socketSetResult) <= 0)
		return needUpdate;

	// Check if any pending connection available.
	if (m_listenSocket->select(true, false, false, 0))
	{
		// Accept new connection.
		Ref< net::TcpSocket > socket = m_listenSocket->accept();
		if (socket)
		{
			// Target must send it's identifier upon connection.
			net::SocketStream ss(socket, true, false);
			Ref< TargetID > targetId = BinarySerializer(&ss).readObject< TargetID >();
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
					needUpdate |= true;
					log::info << L"New target connection accepted; ID " << targetId->getId().format() << Endl;
				}
				else
				{
					// Unknown target; refusing connection.
					socket->close();
					log::error << L"Unknown target ID " << targetId->getId().format() << L"; connection refused" << Endl;
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

	return needUpdate;
}

	}
}
