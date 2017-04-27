/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
