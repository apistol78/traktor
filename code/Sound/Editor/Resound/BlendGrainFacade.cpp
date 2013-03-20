#include "I18N/Text.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Editor/Resound/BlendGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BlendGrainFacade", BlendGrainFacade, IGrainFacade)

int32_t BlendGrainFacade::getImage(const IGrainData* grain) const
{
	return 4;
}

std::wstring BlendGrainFacade::getText(const IGrainData* grain) const
{
	return i18n::Text(L"RESOUND_BLEND_GRAIN_TEXT");
}

bool BlendGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const BlendGrainData* blendGrain = checked_type_cast< const BlendGrainData*, false >(grain);
	outProperties.insert(blendGrain->getId());
	return true;
}

bool BlendGrainFacade::canHaveChildren() const
{
	return false;
}

bool BlendGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool BlendGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool BlendGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	BlendGrainData* blendGrain = checked_type_cast< BlendGrainData*, false >(grain);
	outChildren.push_back(blendGrain->getGrains()[0]);
	outChildren.push_back(blendGrain->getGrains()[1]);
	return true;
}

	}
}
