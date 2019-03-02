#pragma once

#include "Sound/Editor/Resound/IGrainFacade.h"

namespace traktor
{
	namespace sound
	{

class TriggerGrainFacade : public IGrainFacade
{
	T_RTTI_CLASS;

public:
	virtual ui::Widget* createView(IGrainData* grain, ui::Widget* parent) override final;

	virtual int32_t getImage(const IGrainData* grain) const override final;

	virtual std::wstring getText(const IGrainData* grain) const override final;

	virtual bool getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const override final;

	virtual bool canHaveChildren() const override final;

	virtual bool addChild(IGrainData* parentGrain, IGrainData* childGrain) override final;

	virtual bool removeChild(IGrainData* parentGrain, IGrainData* childGrain) override final;

	virtual bool getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren) override final;
};

	}
}

