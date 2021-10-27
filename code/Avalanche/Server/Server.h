#pragma once

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

    namespace net
    {

class DiscoveryManager;
class TcpSocket;

    }

    namespace avalanche
    {

class Connection;
class Dictionary;
class Peer;

class T_DLLCLASS Server : public Object
{
    T_RTTI_CLASS;

public:
    constexpr static int32_t c_majorVersion = 4;
    constexpr static int32_t c_minorVersion = 0;

    bool create(const PropertyGroup* settings);

    void destroy();

    bool update();

    size_t getConnectionCount() const { return m_connections.size(); }

private:
    Ref< net::TcpSocket > m_serverSocket;
    RefArray< Connection > m_connections;
    Ref< net::DiscoveryManager > m_discoveryManager;
    RefArray< Peer > m_peers;
    Ref< Dictionary > m_dictionary;
    Guid m_instanceId;
    bool m_master = false;
};

    }
}
