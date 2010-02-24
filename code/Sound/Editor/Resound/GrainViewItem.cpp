#include "I18N/Format.h"
#include "Sound/Editor/Resound/GrainViewItem.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/SequenceGrain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainViewItem", GrainViewItem, Object)

GrainViewItem::GrainViewItem(IGrain* grain)
:	m_parent(0)
,	m_grain(grain)
{
}

GrainViewItem::GrainViewItem(GrainViewItem* parent, IGrain* grain)
:	m_parent(parent)
,	m_grain(grain)
{
}

GrainViewItem* GrainViewItem::getParent() const
{
	return m_parent;
}

IGrain* GrainViewItem::getGrain() const
{
	return m_grain;
}

int32_t GrainViewItem::getImage() const
{
	if (is_a< PlayGrain >(m_grain))
		return 0;
	else if (is_a< SequenceGrain >(m_grain))
		return 1;
	else if (is_a< RepeatGrain >(m_grain))
		return 2;
	else if (is_a< RandomGrain >(m_grain))
		return 3;
	else if (is_a< MuteGrain >(m_grain))
		return 4;
	else
		return 5;
}

std::wstring GrainViewItem::getText() const
{
	if (const PlayGrain* playGrain = dynamic_type_cast< const PlayGrain* >(m_grain))
		return i18n::Format(L"RESOUND_PLAY_GRAIN_TEXT", int32_t((playGrain->getGain() + 1.0f) * 100));
	else if (const RepeatGrain* repeatGrain = dynamic_type_cast< const RepeatGrain* >(m_grain))
		return i18n::Format(L"RESOUND_REPEAT_GRAIN_TEXT", int32_t(repeatGrain->getCount()));
	else if (const MuteGrain* muteGrain = dynamic_type_cast< const MuteGrain* >(m_grain))
		return i18n::Format(L"RESOUND_MUTE_GRAIN_TEXT", int32_t(muteGrain->getDuration() * 1000));
	else
		return L"";
}

	}
}
