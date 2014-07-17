#include "I18N/Format.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Editor/Resound/EnvelopeGrainFacade.h"
#include "Ui/Event.h"
#include "Ui/Custom/Envelope/DefaultEnvelopeEvaluator.h"
#include "Ui/Custom/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Custom/Envelope/EnvelopeControl.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const Color4ub c_rangeColors[] =
{
	Color4ub(255, 200, 200, 100),
	Color4ub(200, 255, 200, 100),
	Color4ub(200, 200, 255, 100),
	Color4ub(255, 255, 200, 100),
	Color4ub(255, 200, 255, 100),
	Color4ub(200, 255, 255, 100)
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EnvelopeGrainFacade", EnvelopeGrainFacade, IGrainFacade)

ui::Widget* EnvelopeGrainFacade::createView(IGrainData* grain, ui::Widget* parent) const
{
	EnvelopeGrainData* envelopeGrain = checked_type_cast< EnvelopeGrainData*, false >(grain);
	const float* levels = envelopeGrain->getLevels();

	Ref< ui::custom::EnvelopeControl > envelopeControl = new ui::custom::EnvelopeControl();
	envelopeControl->create(parent, new ui::custom::DefaultEnvelopeEvaluator< HermiteEvaluator >(), 0.0f, 1.0f, ui::WsDoubleBuffer);
	
	envelopeControl->insertKey(new ui::custom::EnvelopeKey(0.0f, levels[0], true));
	envelopeControl->insertKey(new ui::custom::EnvelopeKey(envelopeGrain->getMid(), levels[1], false));
	envelopeControl->insertKey(new ui::custom::EnvelopeKey(1.0f, levels[2], true));

	const std::vector< EnvelopeGrainData::GrainData >& grainData = envelopeGrain->getGrains();
	for (uint32_t i = 0; i < uint32_t(grainData.size()); ++i)
	{
		const EnvelopeGrainData::GrainData& gd = grainData[i];
		envelopeControl->addRange(
			c_rangeColors[i % sizeof_array(c_rangeColors)],
			gd.in - gd.easeIn,
			gd.in,
			gd.out,
			gd.out + gd.easeOut
		);
	}

	envelopeControl->setData(L"GRAIN", grain);
	//envelopeControl->addEventHandler< ui::custom::EnvelopeContentChangeEvent >(this, &EnvelopeGrainFacade::eventEnvelopeChange);

	return envelopeControl;
}

int32_t EnvelopeGrainFacade::getImage(const IGrainData* grain) const
{
	return 5;
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

void EnvelopeGrainFacade::eventEnvelopeChange(ui::custom::EnvelopeContentChangeEvent* event)
{
	ui::custom::EnvelopeControl* envelopeControl = checked_type_cast< ui::custom::EnvelopeControl* >(event->getSender());

	EnvelopeGrainData* envelopeGrain = envelopeControl->getData< EnvelopeGrainData >(L"GRAIN");
	T_ASSERT (envelopeGrain);

	const RefArray< ui::custom::EnvelopeKey >& keys = envelopeControl->getKeys();
	T_ASSERT (keys.size() == 3);

	float levels[] =
	{
		keys[0]->getValue(),
		keys[1]->getValue(),
		keys[2]->getValue()
	};
	envelopeGrain->setLevels(levels);
	envelopeGrain->setMid(keys[1]->getT());
}

	}
}
