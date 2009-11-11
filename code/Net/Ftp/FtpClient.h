#ifndef traktor_net_FtpClient_H
#define traktor_net_FtpClient_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class File;
class Stream;
	
	namespace net
	{

class SocketAddressIPv4;
class SocketAddressIPv6;
class TcpSocket;
	
class T_DLLCLASS FtpClient : public Object
{
	T_RTTI_CLASS(FtpClient)
	
public:
	bool connect(const SocketAddressIPv4& socketAddress);

	bool connect(const SocketAddressIPv6& socketAddress);
	
	void disconnect();
	
	bool login(const std::wstring& user, const std::wstring& pwd);
	
	void logout();
	
	void setCurrentDirectory(const std::wstring& cd);
	
	std::wstring getCurrentDirectory();
	
	bool getFileList(RefArray< File >& outFiles);
	
	Ref< Stream > open(const std::wstring& fileName);

private:
	Ref< TcpSocket > m_socket;
	Ref< Stream > m_commandStream;
};
	
	}
}

#endif	// traktor_net_FtpClient_H
