#ifndef traktor_net_Batch_H
#define traktor_net_Batch_H

#include "Core/Misc/Endian.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace net
	{

#if defined(T_BIG_ENDIAN)
template < typename P >
P netEndian(const P& p) { P np = p; swap8in64(np); return np; }
#else
template < typename P >
const P& netEndian(const P& p) { return p; }
#endif

template < typename P1 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1)
{
#pragma pack(1)
	struct { P1 m1; } buf = { netEndian(p1) };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1, typename P2 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; } buf = { netEndian(p1), netEndian(p2) };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1, typename P2, typename P3 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2, const P3& p3)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; P3 m3; } buf = { netEndian(p1), netEndian(p2), netEndian(p3) };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1)
{
#pragma pack(1)
	struct { P1 m1; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = netEndian(buf.m1); return result; }
	return -1;
#pragma pack()
}

template < typename P1, typename P2 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = netEndian(buf.m1); p2 = netEndian(buf.m2); return result; }
	return -1;
#pragma pack()
}

template < typename P1, typename P2, typename P3 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2, P3& p3)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; P3 m3; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = netEndian(buf.m1); p2 = netEndian(buf.m2); p3 = netEndian(buf.m3); return result; }
	return -1;
#pragma pack()
}

	}
}

#endif	// traktor_net_Batch_H
