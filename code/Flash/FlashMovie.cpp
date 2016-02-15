#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Flash/FlashBitmap.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionGlobal.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashMovie", 0, FlashMovie, ISerializable)

FlashMovie::FlashMovie()
{
}

FlashMovie::FlashMovie(const Aabb2& frameBounds, FlashSprite* movieClip)
:	m_frameBounds(frameBounds)
,	m_movieClip(movieClip)
{
}

void FlashMovie::defineFont(uint16_t fontId, FlashFont* font)
{
	m_fonts[fontId] = font;
}

void FlashMovie::defineBitmap(uint16_t bitmapId, FlashBitmap* bitmap)
{
	m_bitmaps[bitmapId] = bitmap;
}

void FlashMovie::defineSound(uint16_t soundId, FlashSound* sound)
{
	m_sounds[soundId] = sound;
}

void FlashMovie::defineCharacter(uint16_t characterId, FlashCharacter* character)
{
	m_characters[characterId] = character;
}

void FlashMovie::setExport(const std::string& name, uint16_t exportId)
{
	m_exports[name] = exportId;
}

Ref< FlashSpriteInstance > FlashMovie::createMovieClipInstance(const IFlashMovieLoader* movieLoader) const
{
	Ref< FlashDictionary > dictionary = new FlashDictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	Ref< ActionContext > context = new ActionContext(this, movieLoader);

	Ref< ActionGlobal > global = new ActionGlobal(context);
	context->setGlobal(global);

	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
		m_movieClip->createInstance(context, dictionary, 0, "", Matrix33::identity(), 0, 0)
	);

	global->setMember("_root", ActionValue(spriteInstance->getAsObject(context)));
	global->setMember("_level0", ActionValue(spriteInstance->getAsObject(context)));

	return spriteInstance;
}

Ref< FlashSpriteInstance > FlashMovie::createExternalMovieClipInstance(FlashSpriteInstance* containerInstance, int32_t depth) const
{
	Ref< FlashDictionary > dictionary = new FlashDictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	// Create instance of external movie.
	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
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

	// Add instance to container's display list.
	FlashDisplayList& displayList = containerInstance->getDisplayList();
	displayList.showObject(
		depth,
		0,
		spriteInstance,
		true
	);

	return spriteInstance;
}

Ref< FlashSpriteInstance > FlashMovie::createExternalSpriteInstance(FlashSpriteInstance* containerInstance, const std::string& characterName, int32_t depth) const
{
	// Get exported character identifier.
	SmallMap< std::string, uint16_t >::const_iterator i = m_exports.find(characterName);
	if (i == m_exports.end())
		return 0;

	uint16_t characterId = i->second;

	// Get exported character.
	SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator j = m_characters.find(characterId);
	if (j == m_characters.end())
		return 0;

	// Get exported sprite.
	const FlashSprite* sprite = dynamic_type_cast< const FlashSprite* >(j->second);
	if (!sprite)
		return 0;

	Ref< FlashDictionary > dictionary = new FlashDictionary();
	dictionary->m_fonts = m_fonts;
	dictionary->m_bitmaps = m_bitmaps;
	dictionary->m_sounds = m_sounds;
	dictionary->m_characters = m_characters;
	dictionary->m_exports = m_exports;

	// Need to create movie instance as it's initialization scripts creates the AS classes.
	Ref< FlashCharacterInstance > movieInstance = m_movieClip->createInstance(
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
	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
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

	// Add instance to container's display list.
	FlashDisplayList& displayList = containerInstance->getDisplayList();
	displayList.showObject(
		depth,
		0,
		spriteInstance,
		true
	);

	movieInstance->destroy();
	return spriteInstance;
}

void FlashMovie::serialize(ISerializer& s)
{
	s >> MemberAabb2(L"frameBounds", m_frameBounds);
	s >> MemberRef< FlashSprite >(L"movieClip", m_movieClip);
	s >> MemberSmallMap< uint16_t, Ref< FlashFont >, Member< uint16_t >, MemberRef< FlashFont > >(L"fonts", m_fonts);
	s >> MemberSmallMap< uint16_t, Ref< FlashBitmap >, Member< uint16_t >, MemberRef< FlashBitmap > >(L"bitmaps", m_bitmaps);
	s >> MemberSmallMap< uint16_t, Ref< FlashSound >, Member< uint16_t >, MemberRef< FlashSound > >(L"sounds", m_sounds);
	s >> MemberSmallMap< uint16_t, Ref< FlashCharacter >, Member< uint16_t >, MemberRef< FlashCharacter > >(L"characters", m_characters);
	s >> MemberSmallMap< std::string, uint16_t >(L"exports", m_exports);
}

	}
}
