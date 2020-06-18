#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
    namespace net
    {

class DiscoveryManager;
class TcpSocket;

    }

    namespace remote
    {

class Server : public Object
{
    T_RTTI_CLASS;

public:
	Server();

    bool create(const std::wstring& scratchPath, const std::wstring& keyword, bool verbose);

    void destroy();

    bool update();

    const std::wstring& getScratchPath() const { return m_scratchPath; }

private:
    Ref< net::TcpSocket > m_serverSocket;
    Ref< net::DiscoveryManager > m_discoveryManager;
	int32_t m_listenPort;
    std::wstring m_hostName;
    std::wstring m_scratchPath;
    std::wstring m_keyword;
    std::map< std::wstring, uint32_t > m_fileHashes;
    bool m_verbose;

    uint8_t handleDeploy(net::TcpSocket* clientSocket);

    uint8_t handleLaunchProcess(net::TcpSocket* clientSocket);

    void processClient(net::TcpSocket* clientSocket);
};

    }
}