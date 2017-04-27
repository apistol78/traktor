/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_PlayGrainFacade_H
#define traktor_sound_PlayGrainFacade_H

#include "Sound/Editor/Resound/IGrainFacade.h"

namespace traktor
{
	namespace sound
	{

class PlayGrainFacade : public IGrainFacade
{
	T_RTTI_CLASS;

public:
	virtual ui::Widget* createView(IGrainData* grain, ui::Widget* parent) T_OVERRIDE T_FINAL;

	virtual int32_t getImage(const IGrainData* grain) const T_OVERRIDE T_FINAL;

	virtual std::wstring getText(const IGrainData* grain) const T_OVERRIDE T_FINAL;

	virtual bool getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const T_OVERRIDE T_FINAL;

	virtual bool canHaveChildren() const T_OVERRIDE T_FINAL;

	virtual bool addChild(IGrainData* parentGrain, IGrainData* childGrain) T_OVERRIDE T_FINAL;

	virtual bool removeChild(IGrainData* parentGrain, IGrainData* childGrain) T_OVERRIDE T_FINAL;

	virtual bool getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_sound_PlayGrainFacade_H
