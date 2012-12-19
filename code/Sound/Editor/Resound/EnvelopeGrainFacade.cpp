#include "I18N/Format.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Editor/Resound/EnvelopeGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EnvelopeGrainFacade", EnvelopeGrainFacade, IGrainFacade)

int32_t EnvelopeGrainFacade::getImage(const IGrainData* grain) const
{
	return 1;
}

std::wstring EnvelopeGrainFacade::getText(const IGrainData* grain) const
{
	uint32_t count = checked_type_cast< const EnvelopeGrainData* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_ENVELOPE_GRAIN_TEXT", int32_t(count));
}

bool EnvelopeGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const EnvelopeGrainData* envelopeGrain = checked_type_cast< const EnvelopeGrainData*, false >(grain);
	outProperties.insert(envelopeGrain->getId());
	return true;
}

bool EnvelopeGrainFacade::canHaveChildren() const
{
	return true;
}

bool EnvelopeGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< EnvelopeGrainData*, false >(parentGrain)->addGrain(childGrain, 0.0f, 1.0f, 0.2f, 0.2f);
	return true;
}

bool EnvelopeGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< EnvelopeGrainData*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool EnvelopeGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	const std::vector< EnvelopeGrainData::GrainData >& grains = checked_type_cast< EnvelopeGrainData*, false >(grain)->getGrains();
	for (std::vector< EnvelopeGrainData::GrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
		outChildren.push_back(i->grain);
	return true;
}

	}
}
