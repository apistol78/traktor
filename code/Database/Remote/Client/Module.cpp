/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Database/Remote/Client/RemoteDatabase.h"

namespace traktor
{
	namespace db
	{

extern "C" void __module__Traktor_Database_Remote_Client()
{
	T_FORCE_LINK_REF(RemoteDatabase);
}

	}
}

#endif
