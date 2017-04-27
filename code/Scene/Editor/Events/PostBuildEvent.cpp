/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Scene/Editor/Events/PostBuildEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.PostBuildEvent", PostBuildEvent, ui::Event)

PostBuildEvent::PostBuildEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}