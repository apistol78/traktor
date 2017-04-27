/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/SparkClassFactory.h"
#	include "Spark/Script/ScriptComponentData.h"
#	include "Spark/Sound/SoundComponentData.h"

namespace traktor
{
	namespace spark
	{

extern "C" void __module__Traktor_Spark()
{
	T_FORCE_LINK_REF(SparkClassFactory);
	T_FORCE_LINK_REF(ScriptComponentData);
	T_FORCE_LINK_REF(SoundComponentData);
}

	}
}

#endif
