/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_IDiscoveryMessage_H
#define traktor_net_IDiscoveryMessage_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace net
	{

class IDiscoveryMessage : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_net_IDiscoveryMessage_H
