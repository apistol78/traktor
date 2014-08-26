#ifndef traktor_net_NetworkTypes_H
#define traktor_net_NetworkTypes_H

#include "Core/Config.h"

namespace traktor
{
	namespace net
	{

enum { MaxDataSize = 1024 };
enum { MaxPeers = 32 };

typedef uint64_t net_handle_t;	//!< Globally unique handle.

	}
}

#endif	// traktor_net_NetworkTypes_H
