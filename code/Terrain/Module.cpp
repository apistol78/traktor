/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Terrain/TerrainClassFactory.h"
#	include "Terrain/RubbleLayerData.h"
#	include "Terrain/UndergrowthLayerData.h"

namespace traktor
{
	namespace terrain
	{

extern "C" void __module__Traktor_Terrain()
{
	T_FORCE_LINK_REF(TerrainClassFactory);
	T_FORCE_LINK_REF(RubbleLayerData);
	T_FORCE_LINK_REF(UndergrowthLayerData);
}

	}
}

#endif
