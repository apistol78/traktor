/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Flash/Bitmap.h"
#include "Flash/Dictionary.h"
#include "Flash/Font.h"
#include "Flash/Movie.h"
#include "Flash/Sound.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionGlobal.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Movie", 0, Movie, ISerializable)

Movie::Movie()
{
}

Movie::Movie(const Aabb2& frameBounds, Sprite* movieClip)
:	m_frameBounds(frameBounds)
,	m_movieClip(movieClip)
{
}

void Movie::defineFont(uint16_t fontId, Font* font)
{
	m_fonts[fontId] = font;
}

void Movie::defineBitmap(uint16_t bitmapId, Bitmap* bitmap)
{
	m_bitmaps[bitmapId] = bitmap;
}

void Movie::defineSound(uint16_t soundId, Sound* sound)
{
	m_sounds[soundId] = sound;
}

void Movie::defineCharacter(uint16_t characterId, Character* character)
{
	m_characters[characterId] = character;
}

void Movie::setExport(const std::string& name, uint16_t exportId)
{
	m_exports[name] = exportId;
}

Ref< SpriteInstance > Movie::createMovieClipInstance(const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader) const
{
	Ref< Dictionary > dictionary = new Dictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	Ref< ActionContext > context = new ActionContext(this, characterFactory, movieLoader);

	Ref< ActionGlobal > global = new ActionGlobal(context);
	context->setGlobal(global);

	Ref< SpriteInstance > spriteInstance = checked_type_cast< SpriteInstance*, false >(
		m_movieClip->createInstance(context, dictionary, 0, "", Matrix33::identity(), 0, 0)
	);
	if (!spriteInstance)
		return 0;

	global->setMember("_root", ActionValue(spriteInstance->getAsObject(context)));
	global->setMember("_level0", ActionValue(spriteInstance->getAsObject(context)));

	return spriteInstance;
}

Ref< SpriteInstance > Movie::createExternalMovieClipInstance(SpriteInstance* containerInstance, int32_t depth) const
{
	if (!containerInstance)
		return 0;

	Ref< Dictionary > dictionary = new Dictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	// Create instance of external movie.
	Ref< SpriteInstance > spriteInstance = checked_type_cast< SpriteInstance*, false >(
		m_movieClip->createInstance(
			containerInstance->getContext(),
			dictionary,
			containerInstance,
			"",
			Matrix33::identity(),
			0,
			0
		)
	);
	if (!spriteInstance)
		return 0;

	// Add instance to container's display list.
	DisplayList& displayList = containerInstance->getDisplayList();
	displayList.showObject(
		depth,
		0,
		spriteInstance,
		true
	);

	return spriteInstance;
}

Ref< SpriteInstance > Movie::createExternalSpriteInstance(SpriteInstance* containerInstance, const std::string& characterName, int32_t depth) const
{
	if (!containerInstance)
		return 0;

	// Get exported character identifier.
	SmallMap< std::string, uint16_t >::const_iterator i = m_exports.find(characterName);
	if (i == m_exports.end())
		return 0;

	uint16_t characterId = i->second;

	// Get exported character.
	SmallMap< uint16_t, Ref< Character > >::const_iterator j = m_characters.find(characterId);
	if (j == m_characters.end())
		return 0;

	// Get exported sprite.
	const Sprite* sprite = dynamic_type_cast< const Sprite* >(j->second);
	if (!sprite)
		return 0;

	Ref< Dictionary > dictionary = new Dictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	// Need to create movie instance as it's initialization scripts creates the AS classes.
	Ref< CharacterInstance > movieInstance = m_movieClip->createInstance(
		containerInstance->getContext(),
		dictionary,
		containerInstance,
		"",
		Matrix33::identity(),
		0,
		0
	);
	if (!movieInstance)
		return 0;

	// Create instance of external sprite.
	Ref< SpriteInstance > spriteInstance = checked_type_cast< SpriteInstance*, false >(
		sprite->createInstance(
			containerInstance->getContext(),
			dictionary,
			containerInstance,
			"",
			Matrix33::identity(),
			0,
			0
		)
	);
	if (!spriteInstance)
		return 0;

	// Add instance to container's display list.
	DisplayList& displayList = containerInstance->getDisplayList();
	displayList.showObject(
		depth,
		0,
		spriteInstance,
		true
	);

	movieInstance->destroy();
	return spriteInstance;
}

void Movie::serialize(ISerializer& s)
{
	s >> MemberAabb2(L"frameBounds", m_frameBounds);
	s >> MemberRef< Sprite >(L"movieClip", m_movieClip);
	s >> MemberSmallMap< uint16_t, Ref< Font >, Member< uint16_t >, MemberRef< Font > >(L"fonts", m_fonts);
	s >> MemberSmallMap< uint16_t, Ref< Bitmap >, Member< uint16_t >, MemberRef< Bitmap > >(L"bitmaps", m_bitmaps);
	s >> MemberSmallMap< uint16_t, Ref< Sound >, Member< uint16_t >, MemberRef< Sound > >(L"sounds", m_sounds);
	s >> MemberSmallMap< uint16_t, Ref< Character >, Member< uint16_t >, MemberRef< Character > >(L"characters", m_characters);
	s >> MemberSmallMap< std::string, uint16_t >(L"exports", m_exports);
}

	}
}
