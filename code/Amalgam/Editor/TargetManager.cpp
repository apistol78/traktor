/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/TargetID.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Editor/IEditor.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetManager", TargetManager, Object)

TargetManager::TargetManager(editor::IEditor *editor, script::IScriptDebuggerSessions *targetDebuggerSessions)
:	m_editor(editor)
,	m_targetDebuggerSessions(targetDebuggerSessions)
,	m_port(0)
{
}

bool TargetManager::create()
{
	// Create our server socket.
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(0)))
	{
		log::error << L"Failed to create target manager; Unable to bind socket." << Endl;
		return false;
	}

	if (!m_listenSocket->listen())
	{
		log::error << L"Failed to create target manager; Unable to listen on socket." << Endl;
		return false;
	}

	m_port = dynamic_type_cast< net::SocketAddressIPv4* >(m_listenSocket->getLocalAddress())->getPort();

	log::info << L"Target manager @" << m_port << L" created" << Endl;
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

	m_targetDebuggerSessions = 0;
	m_editor = 0;
}

void TargetManager::addInstance(TargetInstance* targetInstance)
{
	m_instances.push_back(targetInstance);
}

void TargetManager::removeInstance(TargetInstance* targetInstance)
{
	m_instances.remove(targetInstance);
}

void TargetManager::removeAllInstances()
{
	m_instances.clear();
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
		RefArray< TargetConnection> connections = (*i)->getConnections();
		for (RefArray< TargetConnection >::const_iterator j = connections.begin(); j != connections.end(); ++j)
		{
			net::BidirectionalObjectTransport* transport = (*j)->getTransport();
			T_ASSERT (transport);

			if (transport->getSocket())
				socketSet.add(transport->getSocket());
		}
	}

	// Wait on all sockets.
	if (socketSet.select(true, false, false, 0, socketSetResult) <= 0)
		return needUpdate;

	// Check if any pending connection available.
	if (m_listenSocket->select(true, false, false, 0))
	{
		// Accept new connection.
		Ref< net::TcpSocket > socket = m_listenSocket->accept();
		if (socket)
		{
			Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(socket);
			socket->setNoDelay(true);

			// Target must send it's identifier upon connection.
			Ref< TargetID > targetId;
			if (transport->recv< TargetID >(1000, targetId) == net::BidirectionalObjectTransport::RtSuccess)
			{
				T_ASSERT (targetId);

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
					// Determine log name; if multiple running instances with same name then add number.
					int32_t running = 0;
					const RefArray< TargetConnection >& connections = instance->getConnections();
					for (RefArray< TargetConnection >::const_iterator i = connections.begin(); i != connections.end(); ++i)
					{
						if ((*i)->getName() == targetId->getName())
							++running;
					}

					std::wstring logName = targetId->getName();
					if (running > 0)
						logName += L" (" + toString(running) + L")";

					Ref< ILogTarget > targetLog = m_editor->createLogTarget(logName);

					// Create connection object and add to instance.
					instance->addConnection(new TargetConnection(targetId->getName(), transport, targetLog, m_targetDebuggerSessions));
					needUpdate |= true;
					log::info << L"New target connection accepted; ID " << targetId->getId().format() << Endl;
				}
				else
				{
					// Unknown target; refusing connection.
					log::error << L"Unknown target ID " << targetId->getId().format() << L" from " << targetId->getName() << L"; connection refused" << Endl;
				}
			}
			else
			{
				// Invalid target ID message; refusing connection.
				log::error << L"Invalid target ID message; connection refused" << Endl;
			}
		}
	}

	return needUpdate;
}

	}
}
