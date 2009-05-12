#include "Flash/FlashFrame.h"
#include "Flash/Action/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashFrame", FlashFrame, ActionObject)

FlashFrame::FlashFrame()
:	ActionObject(AsObject::getInstance())
,	m_backgroundColorChange(false)
{
	m_backgroundColor.red =
	m_backgroundColor.green =
	m_backgroundColor.blue =
	m_backgroundColor.alpha = 255;
}

void FlashFrame::setLabel(const std::wstring& label)
{
	m_label = label;
}

const std::wstring& FlashFrame::getLabel() const
{
	return m_label;
}

void FlashFrame::changeBackgroundColor(const SwfColor& backgroundColor)
{
	m_backgroundColor = backgroundColor;
	m_backgroundColorChange = true;
}

bool FlashFrame::hasBackgroundColorChanged() const
{
	return m_backgroundColorChange;
}

const SwfColor& FlashFrame::getBackgroundColor() const
{
	return m_backgroundColor;
}

void FlashFrame::placeObject(const PlaceObject& placeObject)
{
	T_ASSERT (placeObject.depth > 0);
	m_placeObjects[placeObject.depth] = placeObject;
}

void FlashFrame::removeObject(const RemoveObject& removeObject)
{
	T_ASSERT (removeObject.depth > 0);
	m_removeObjects[removeObject.depth] = removeObject;
}

void FlashFrame::addActionScript(ActionScript* actionScript)
{
	m_actionScripts.push_back(actionScript);
}

const std::map< uint16_t, FlashFrame::PlaceObject >& FlashFrame::getPlaceObjects() const
{
	return m_placeObjects;
}

const std::map< uint16_t, FlashFrame::RemoveObject >& FlashFrame::getRemoveObjects() const
{
	return m_removeObjects;
}

const RefArray< ActionScript >& FlashFrame::getActionScripts() const
{
	return m_actionScripts;
}

	}
}
