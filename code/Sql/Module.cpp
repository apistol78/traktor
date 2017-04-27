/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sql/SqlClassFactory.h"

namespace traktor
{
	namespace sql
	{

extern "C" void __module__Traktor_Sql()
{
	T_FORCE_LINK_REF(SqlClassFactory);
}

	}
}

#endif
