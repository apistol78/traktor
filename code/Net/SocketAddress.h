#ifndef traktor_net_SocketAddress_H
#define traktor_net_SocketAddress_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! \brief Socket address.
 * \ingroup Net
 */
class T_DLLCLASS SocketAddress : public Object
{
	T_RTTI_CLASS(SocketAddress)
};
	
	}
}

#endif	// traktor_net_SocketAddress_H
