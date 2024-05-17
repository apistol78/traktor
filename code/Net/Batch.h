/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/Endian.h"
#include "Net/TcpSocket.h"

namespace traktor::net
{

#if defined(T_BIG_ENDIAN)
template < typename P >
P netEndian(const P& p) { P np = p; swap8in64(np); return np; }
#else
template < typename P >
const P& netEndian(const P& p) { return p; }
#endif

template < typename P >
void endianAwarePack(uint8_t* data, const P& p) { *(P*)data = netEndian(p); }

template < typename P1 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1)
{
	uint8_t data[sizeof(P1)];
	endianAwarePack(data, p1);
	return socket->send(data, sizeof(data));
}

template < typename P1, typename P2 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2)
{
	uint8_t data[sizeof(P1) + sizeof(P2)];
	endianAwarePack(&data[0], p1);
	endianAwarePack(&data[sizeof(P1)], p2);
	return socket->send(data, sizeof(data));
}

template < typename P1, typename P2, typename P3 >
int32_t sendBatch(net::TcpSocket* socket, const P1& p1, const P2& p2, const P3& p3)
{
	uint8_t data[sizeof(P1) + sizeof(P2) + sizeof(P3)];
	endianAwarePack(&data[0], p1);
	endianAwarePack(&data[sizeof(P1)], p2);
	endianAwarePack(&data[sizeof(P1) + sizeof(P2)], p3);
	return socket->send(data, sizeof(data));
}

template < typename P1 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1)
{
	uint8_t data[sizeof(P1)];
	const int32_t result = socket->recv(data, sizeof(data));
	if (result == sizeof(data))
	{
		p1 = netEndian((P1&)data[0]);
		return result;
	}
	else
		return -1;
}

template < typename P1, typename P2 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2)
{
	uint8_t data[sizeof(P1) + sizeof(P2)];
	const int32_t result = socket->recv(data, sizeof(data));
	if (result == sizeof(data))
	{
		p1 = netEndian((P1&)data[0]);
		p2 = netEndian((P2&)data[sizeof(P1)]);
		return result;
	}
	else
		return -1;
}

template < typename P1, typename P2, typename P3 >
int32_t recvBatch(net::TcpSocket* socket, P1& p1, P2& p2, P3& p3)
{
	uint8_t data[sizeof(P1) + sizeof(P2) + sizeof(P3)];
	const int32_t result = socket->recv(data, sizeof(data));
	if (result == sizeof(data))
	{
		p1 = netEndian((P1&)data[0]);
		p2 = netEndian((P2&)data[sizeof(P1)]);
		p3 = netEndian((P3&)data[sizeof(P1) + sizeof(P2)]);
		return result;
	}
	else
		return -1;
}

}
