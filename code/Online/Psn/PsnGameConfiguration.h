#ifndef traktor_online_PsnGameConfiguration_H
#define traktor_online_PsnGameConfiguration_H

#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class PsnGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_online_PsnGameConfiguration_H
