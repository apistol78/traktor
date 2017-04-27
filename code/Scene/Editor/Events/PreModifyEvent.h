/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_PreModifyEvent_H
#define traktor_scene_PreModifyEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS PreModifyEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	PreModifyEvent(ui::EventSubject* sender);
};

	}
}

#endif	// traktor_scene_PreModifyEvent_H
