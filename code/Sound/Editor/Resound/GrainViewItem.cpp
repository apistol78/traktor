#include "I18N/Format.h"
#include "Sound/Editor/Resound/GrainViewItem.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainViewItem", GrainViewItem, Object)

GrainViewItem::GrainViewItem(GrainViewItem* parent, IGrain* grain, const std::wstring& text, int32_t image)
:	m_parent(parent)
,	m_grain(grain)
,	m_text(text)
,	m_image(image)
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
	return m_image;
}

std::wstring GrainViewItem::getText() const
{
	return m_text;
}

	}
}
