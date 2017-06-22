/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_Socket_H
#define traktor_net_Socket_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

enum IoctlCommand
{
	IccNonBlockingIo,	// FIONBIO
	IccReadPending		// FIONREAD
};

/*! \brief Socket base class.
 * \ingroup Net
 */
class T_DLLCLASS Socket : public Object
{
	T_RTTI_CLASS;

public:
	typedef intptr_t handle_t;

	Socket();

	Socket(handle_t socket_);

	virtual ~Socket();

	/*! \brief Gracefully close socket. */
	virtual void close();
	
	/*! \brief Query state of socket.
	 *
	 * \param read Check for pending reads.
	 * \param write Check pending writes. 
	 * \param except Check for exceptions.
	 * \param timeout Timeout in milliseconds.
	 * \return Non zero if read, write or exception are pending.
	 */
	int select(bool read, bool write, bool except, int timeout);

	/*! \brief Send block of data through socket.
	 *
	 * \param data Pointer to block of data.
	 * \param length Number of bytes.
	 * \return Number of bytes sent, 0 if connection gracefully terminated or -1 connection aborted.
	 */
	int send(const void* data, int length);
	
	/*! \brief Receive block of data.
	 *
	 * \param data Pointer to block of data.
	 * \param length Number of bytes.
	 * \return Number of bytes received, 0 if connection terminated gracefully or -1 if connection aborted.
	 */
	int recv(void* data, int length);

	/*! \brief Send single byte.
	 *
	 * \param byte Byte to send.
	 * \return Number of bytes sent, 0 if connection gracefully terminated or -1 connection aborted.
	 */
	int send(int byte);

	/*! \brief Read single byte.
	 *
	 * \return Byte value or -1 if connection aborted.
	 */
	int recv();
	
	/*! \brief Control IO of socket.
	 *
	 * \param cmd IO command.
	 * \param argp IO command argument(s).
	 * \return Result of IO command.
	 */
	bool ioctl(IoctlCommand cmd, unsigned long* argp);

	/*! \brief Get socket handle.
	 *
	 * \return Socket handle.
	 */
	handle_t handle() const;

protected:
	handle_t m_socket;
};

	}
}

#endif	// traktor_net_Socket_H
