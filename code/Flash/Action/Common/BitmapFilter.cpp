/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/Common/BitmapFilter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.BitmapFilter", BitmapFilter, ActionObjectRelay)

BitmapFilter::BitmapFilter(const char* const prototype)
:	ActionObjectRelay(prototype)
{
}

	}
}
