#ifndef traktor_net_Batch_H
#define traktor_net_Batch_H

namespace traktor
{
	namespace net
	{

class TcpSocket;

template < typename P1 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1)
{
#pragma pack(1)
	struct { P1 m1; } buf = { p1 };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1, typename P2 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; } buf = { p1, p2 };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1, typename P2, typename P3 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2, const P3& p3)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; P3 m3; } buf = { p1, p2, p3 };
	return socket->send(&buf, sizeof(buf));
#pragma pack()
}

template < typename P1 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1)
{
#pragma pack(1)
	struct { P1 m1; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = buf.m1; }
	return result;
#pragma pack()
}

template < typename P1, typename P2 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = buf.m1; p2 = buf.m2; }
	return result;
#pragma pack()
}

template < typename P1, typename P2, typename P3 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2, P3& p3)
{
#pragma pack(1)
	struct { P1 m1; P2 m2; P3 m3; } buf;
	int32_t result = socket->recv(&buf, sizeof(buf));
	if (result == sizeof(buf)) { p1 = buf.m1; p2 = buf.m2; p3 = buf.m3; }
	return result;
#pragma pack()
}

	}
}

#endif	// traktor_net_Batch_H
