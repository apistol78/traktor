#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

}

namespace traktor::net
{

class SocketStream;
class TcpSocket;

}

namespace traktor::avalanche
{

class Dictionary;

class T_DLLCLASS Connection : public Object
{
	T_RTTI_CLASS;

public:
	explicit Connection(Dictionary* dictionary);

	virtual ~Connection();

	bool create(net::TcpSocket* clientSocket);

	bool update();

private:
	Dictionary* m_dictionary = nullptr;
	Ref< net::TcpSocket > m_clientSocket;
	Ref< net::SocketStream > m_clientStream;
	Thread* m_thread = nullptr;
	std::atomic< bool > m_finished;

	bool process();
};

}
