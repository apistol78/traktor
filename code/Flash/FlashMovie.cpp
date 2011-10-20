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

FlashMovie::FlashMovie(const IActionVM* vm, const SwfRect& frameBounds, FlashSprite* movieClip)
:	m_vm(vm)
,	m_frameBounds(frameBounds)
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
	for (SmallMap< uint16_t, Ref< FlashCharacter > >::iterator i = m_characters.begin(); i != m_characters.end(); ++i)
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
	SmallMap< uint16_t, Ref< FlashFont > >::const_iterator i = m_fonts.find(fontId);
	return i != m_fonts.end() ? i->second.ptr() : 0;
}

const FlashBitmap* FlashMovie::getBitmap(uint16_t bitmapId) const
{
	SmallMap< uint16_t, Ref< FlashBitmap > >::const_iterator i = m_bitmaps.find(bitmapId);
	return i != m_bitmaps.end() ? i->second.ptr() : 0;
}

const FlashCharacter* FlashMovie::getCharacter(uint16_t characterId) const
{
	SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator i = m_characters.find(characterId);
	return i != m_characters.end() ? i->second.ptr() : 0;
}

void FlashMovie::setExport(const std::string& name, uint16_t exportId)
{
	m_exports[name] = exportId;
}

bool FlashMovie::getExportId(const std::string& name, uint16_t& outExportId) const
{
	SmallMap< std::string, uint16_t >::const_iterator i = m_exports.find(name);
	if (i != m_exports.end())
	{
		outExportId = i->second;
		return true;
	}
	return false;
}

bool FlashMovie::getExportName(uint16_t exportId, std::string& outName) const
{
	for (SmallMap< std::string, uint16_t >::const_iterator i = m_exports.begin(); i != m_exports.end(); ++i)
	{
		if (i->second == exportId)
		{
			outName = i->first;
			return true;
		}
	}
	return false;
}

Ref< FlashSpriteInstance > FlashMovie::createMovieClipInstance() const
{
	Ref< ActionContext > context = new ActionContext(m_vm, this);

	Ref< ActionGlobal > global = new ActionGlobal(context);
	context->setGlobal(global);

	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
		m_movieClip->createInstance(context, 0, "")
	);

	global->setMember("_root", ActionValue(spriteInstance->getAsObject(context)));
	global->setMember("_level0", ActionValue(spriteInstance->getAsObject(context)));

	return spriteInstance;
}

Ref< FlashSpriteInstance > FlashMovie::createExternalMovieClipInstance(FlashSpriteInstance* containerInstance) const
{
	// Create context; share VM and global.
	Ref< ActionContext > outerContext = containerInstance->getContext();

	Ref< ActionContext > context = new ActionContext(outerContext->getVM(), this);
	context->setGlobal(outerContext->getGlobal());

	// Create instance of external movie.
	Ref< FlashSpriteInstance > spriteInstance = checked_type_cast< FlashSpriteInstance*, false >(
		m_movieClip->createInstance(context, containerInstance, "")
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
