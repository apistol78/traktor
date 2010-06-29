#include "Flash/FlashMovie.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashBitmap.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/Avm1/ActionGlobal.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovie", FlashMovie, Object)

FlashMovie::FlashMovie(const SwfRect& frameBounds, FlashSprite* movieClip)
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

void FlashMovie::defineCharacter(uint16_t characterId, FlashCharacter* character)
{
	m_characters[characterId] = character;
}

void FlashMovie::removeCharacter(FlashCharacter* character)
{
	for (std::map< uint16_t, Ref< FlashCharacter > >::iterator i = m_characters.begin(); i != m_characters.end(); ++i)
	{
		if (i->second == character)
		{
			m_characters.erase(i);
			break;
		}
	}
}

const FlashFont* FlashMovie::getFont(uint16_t fontId) const
{
	std::map< uint16_t, Ref< FlashFont > >::const_iterator i = m_fonts.find(fontId);
	return i != m_fonts.end() ? i->second.ptr() : 0;
}

const FlashBitmap* FlashMovie::getBitmap(uint16_t bitmapId) const
{
	std::map< uint16_t, Ref< FlashBitmap > >::const_iterator i = m_bitmaps.find(bitmapId);
	return i != m_bitmaps.end() ? i->second.ptr() : 0;
}

const FlashCharacter* FlashMovie::getCharacter(uint16_t characterId) const
{
	std::map< uint16_t, Ref< FlashCharacter > >::const_iterator i = m_characters.find(characterId);
	return i != m_characters.end() ? i->second.ptr() : 0;
}

void FlashMovie::setExport(const std::wstring& name, uint16_t exportId)
{
	m_exports[name] = exportId;
}

bool FlashMovie::getExportId(const std::wstring& name, uint16_t& outExportId) const
{
	std::map< std::wstring, uint16_t >::const_iterator i = m_exports.find(name);
	if (i != m_exports.end())
	{
		outExportId = i->second;
		return true;
	}
	return false;
}

bool FlashMovie::getExportName(uint16_t exportId, std::wstring& outName) const
{
	for (std::map< std::wstring, uint16_t >::const_iterator i = m_exports.begin(); i != m_exports.end(); ++i)
	{
		if (i->second == exportId)
		{
			outName = i->first;
			return true;
		}
	}
	return false;
}

const SwfRect& FlashMovie::getFrameBounds() const
{
	return m_frameBounds;
}

const FlashSprite* FlashMovie::getMovieClip() const
{
	return m_movieClip;
}

Ref< FlashSpriteInstance > FlashMovie::createMovieClipInstance(const IActionVM* vm) const
{
	Ref< ActionGlobal > global = new ActionGlobal();
	Ref< ActionContext > context = new ActionContext(vm, this, global);
	
	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
		m_movieClip->createInstance(context, 0)
	);

	global->setMember(L"_root", ActionValue(spriteInstance));
	global->setMember(L"_level0", ActionValue(spriteInstance));

	return spriteInstance;
}

Ref< FlashSpriteInstance > FlashMovie::createExternalMovieClipInstance(FlashSpriteInstance* containerInstance) const
{
	// Create context; share VM and global.
	Ref< ActionContext > outerContext = containerInstance->getContext();
	Ref< ActionContext > context = new ActionContext(
		outerContext->getVM(),
		this,
		outerContext->getGlobal()
	);

	// Create instance of external movie.
	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
		m_movieClip->createInstance(context, containerInstance)
	);

	// Add instance to container's display list.
	FlashDisplayList& displayList = containerInstance->getDisplayList();
	displayList.showObject(
		0,
		0,
		spriteInstance,
		true
	);

	return spriteInstance;
}

	}
}
