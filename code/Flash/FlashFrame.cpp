#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashFrame", 1, FlashFrame, ISerializable)

FlashFrame::FlashFrame()
:	m_backgroundColorChange(false)
{
	m_backgroundColor.red =
	m_backgroundColor.green =
	m_backgroundColor.blue =
	m_backgroundColor.alpha = 255;
}

void FlashFrame::setLabel(const std::string& label)
{
	m_label = label;
}

const std::string& FlashFrame::getLabel() const
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

void FlashFrame::addActionScript(const IActionVMImage* actionScript)
{
	m_actionScripts.push_back(actionScript);
}

void FlashFrame::startSound(uint16_t soundId)
{
	m_startSounds.push_back(soundId);
}

const SmallMap< uint16_t, FlashFrame::PlaceObject >& FlashFrame::getPlaceObjects() const
{
	return m_placeObjects;
}

const SmallMap< uint16_t, FlashFrame::RemoveObject >& FlashFrame::getRemoveObjects() const
{
	return m_removeObjects;
}

const AlignedVector< uint16_t >& FlashFrame::getStartSounds() const
{
	return m_startSounds;
}

const RefArray< const IActionVMImage >& FlashFrame::getActionScripts() const
{
	return m_actionScripts;
}

void FlashFrame::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 1);

	s >> Member< std::string >(L"label", m_label);
	s >> Member< bool >(L"backgroundColorChange", m_backgroundColorChange);
	
	if (m_backgroundColorChange)
		s >> MemberSwfColor(L"backgroundColor", m_backgroundColor);

	s >> MemberSmallMap< uint16_t, PlaceObject, Member< uint16_t >, MemberComposite< PlaceObject > >(L"placeObjects", m_placeObjects);
	s >> MemberSmallMap< uint16_t, RemoveObject, Member< uint16_t >, MemberComposite< RemoveObject > >(L"removeObjects", m_removeObjects);
	s >> MemberAlignedVector< uint16_t >(L"startSounds", m_startSounds);
	s >> MemberRefArray< const IActionVMImage >(L"actionScripts", m_actionScripts);
}

void FlashFrame::PlaceObject::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"hasFlags", hasFlags);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasFlags & PfHasBitmapCaching)
		s >> Member< uint8_t >(L"bitmapCaching", bitmapCaching);

	if (hasFlags & PfHasBlendMode)
		s >> Member< uint8_t >(L"blendMode", blendMode);

	if (hasFlags & PfHasActions)
		s >> MemberSmallMap<
			uint32_t,
			Ref< const IActionVMImage >,
			Member< uint32_t >,
			MemberRef< const IActionVMImage >
		>(L"events", events);

	if (hasFlags & PfHasClipDepth)
		s >> Member< uint16_t >(L"clipDepth", clipDepth);

	if (hasFlags & PfHasName)
		s >> Member< std::string >(L"name", name);

	if (hasFlags & PfHasRatio)
		s >> Member< uint16_t >(L"ratio", ratio);

	if (hasFlags & PfHasCxTransform)
		s >> MemberSwfCxTransform(L"cxTransform", cxTransform);

	if (hasFlags & PfHasMatrix)
		s >> Member< Matrix33 >(L"matrix", matrix);

	if (hasFlags & PfHasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);
}

void FlashFrame::RemoveObject::serialize(ISerializer& s)
{
	s >> Member< bool >(L"hasCharacterId", hasCharacterId);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);
}

	}
}
