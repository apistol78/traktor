/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Flash/BitmapImage.h"
#include "Flash/MovieFactory.h"
#include "Flash/MovieFactoryTags.h"
#include "Flash/Movie.h"
#include "Flash/Shape.h"
#include "Flash/Sprite.h"
#include "Flash/Frame.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/Avm1/ActionVM1.h"
#include "Flash/Action/Avm2/ActionVM2.h"

#define T_SHOW_STATISTICS 0

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieFactory", MovieFactory, Object)

MovieFactory::MovieFactory(bool includeAS)
:	m_includeAS(includeAS)
{
	m_tagReaders[TiFileAttributes] = new TagFileAttributes();

	// Setup tag readers.
	m_tagReaders[TiSetBackgroundColor] = new TagSetBackgroundColor();
	m_tagReaders[TiDefineShape] = new TagDefineShape(1);
	m_tagReaders[TiDefineShape2] = new TagDefineShape(2);
	m_tagReaders[TiDefineShape3] = new TagDefineShape(3);
	m_tagReaders[TiDefineShape4] = new TagDefineShape(4);
	m_tagReaders[TiDefineMorphShape] = new TagDefineMorphShape(1);
	m_tagReaders[TiDefineMorphShape2] = new TagDefineMorphShape(2);
	m_tagReaders[TiDefineFont] = new TagDefineFont(1);
	m_tagReaders[TiDefineFont2] = new TagDefineFont(2);
	m_tagReaders[TiDefineFont3] = new TagDefineFont(3);
	m_tagReaders[TiDefineScalingGrid] = new TagDefineScalingGrid();
	m_tagReaders[TiJpegTables] = new TagJpegTables();
	m_tagReaders[TiDefineBitsJpeg] = new TagDefineBitsJpeg(1);
	m_tagReaders[TiDefineBitsJpeg2] = new TagDefineBitsJpeg(2);
	m_tagReaders[TiDefineBitsJpeg3] = new TagDefineBitsJpeg(3);
	m_tagReaders[TiDefineBitsLossLess] = new TagDefineBitsLossLess(1);
	m_tagReaders[TiDefineBitsLossLess2] = new TagDefineBitsLossLess(2);
	m_tagReaders[TiDefineSprite] = new TagDefineSprite();
	m_tagReaders[TiDefineText] = new TagDefineText(1);
	m_tagReaders[TiDefineText2] = new TagDefineText(2);
	m_tagReaders[TiDefineEditText] = new TagDefineEditText();
	//m_tagReaders[TiDefineButton] = new TagDefineButton(1);
	m_tagReaders[TiDefineButton2] = new TagDefineButton(2);
	m_tagReaders[TiDefineSceneAndFrameLabelData] = new TagDefineSceneAndFrameLabelData();
	m_tagReaders[TiPlaceObject] = new TagPlaceObject(1);
	m_tagReaders[TiPlaceObject2] = new TagPlaceObject(2);
	m_tagReaders[TiPlaceObject3] = new TagPlaceObject(3);
	m_tagReaders[TiRemoveObject] = new TagRemoveObject(1);
	m_tagReaders[TiRemoveObject2] = new TagRemoveObject(2);
	m_tagReaders[TiDoAction] = new TagDoAction();
	m_tagReaders[TiExportAssets] = new TagExportAssets();
	m_tagReaders[TiImportAssets] = new TagImportAssets(1);
	m_tagReaders[TiImportAssets2] = new TagImportAssets(2);
	m_tagReaders[TiSymbolClass] = new TagSymbolClass();
	m_tagReaders[TiMetadata] = new TagMetaData();
	
	if (m_includeAS)
		m_tagReaders[TiInitAction] = new TagInitAction();

	m_tagReaders[TiShowFrame] = new TagShowFrame();
	m_tagReaders[TiProtect] = new TagProtect(1);
	m_tagReaders[TiEnableDebugger] = new TagProtect(2);
	m_tagReaders[TiEnableDebugger2] = new TagProtect(3);
	m_tagReaders[TiFrameLabel] = new TagFrameLabel();
	
	if (m_includeAS)
		m_tagReaders[TiDoABC] = new TagDoABC();

	m_tagReaders[TiDefineSound] = new TagDefineSound();
	m_tagReaders[TiStartSound] = new TagStartSound(1);
	m_tagReaders[TiStartSound2] = new TagStartSound(2);

	// Define readers for tags which isn't planed to be implemented.
	m_tagReaders[TiDefineFontInfo] = new TagUnsupported(TiDefineFontInfo);
	m_tagReaders[TiSoundStreamHead] = new TagUnsupported(TiSoundStreamHead);
	m_tagReaders[TiSoundStreamBlock] = new TagUnsupported(TiSoundStreamBlock);
	m_tagReaders[TiSoundStreamHead2] = new TagUnsupported(TiSoundStreamHead2);
	m_tagReaders[TiDebugID] = new TagUnsupported(TiDebugID);
	m_tagReaders[TiDefineFontAlignZones] = new TagUnsupported(TiDefineFontAlignZones);
	m_tagReaders[TiCSMTextSettings] = new TagUnsupported(TiCSMTextSettings);
	m_tagReaders[TiDefineFontName] = new TagUnsupported(TiDefineFontName);
}

Ref< Movie > MovieFactory::createMovie(SwfReader* swf) const
{
	SwfHeader* header = swf->readHeader();
	if (!header)
	{
		log::error << L"Unable to read SWF movie; invalid header" << Endl;
		return 0;
	}

	T_DEBUG(L"SWF movie version " << int32_t(header->version));

	// Create new movie.
	Ref< Sprite > movieClip = new Sprite(0, header->frameRate >> 8);
	Ref< Movie > movie = new Movie(header->frameRect, movieClip);

	// Decode tags.
	Tag::ReadContext context;
	context.version = header->version;
	context.movie = movie;
	context.sprite = movieClip;
	context.frame = new Frame();

	if (m_includeAS)
	{
		context.avm1 = new ActionVM1();
		context.avm2 = new ActionVM2();
	}

	for (bool going = true; going; )
	{
		swf->enterScope();

		SwfTag* tag = swf->readTag();
		if (!tag || tag->id == TiEnd)
			break;

		Ref< Tag > tagReader;
		std::map< uint16_t, Ref< Tag > >::const_iterator i = m_tagReaders.find(tag->id);
		if (i != m_tagReaders.end())
			tagReader = i->second;

		if (tagReader)
		{
			context.tagSize = tag->length;
			context.tagEndPosition = swf->getBitReader().getStream()->tell() + tag->length;
			if (!tagReader->read(swf, context))
			{
				log::error << L"Unable to read SWF; error when reading tag " << int32_t(tag->id) << Endl;
				return 0;
			}
			if (uint32_t(swf->getBitReader().getStream()->tell()) < context.tagEndPosition)
			{
				T_DEBUG(L"Read too few bytes (" << context.tagEndPosition - swf->getBitReader().getStream()->tell() << L" left) in tag " << int32_t(tag->id));
				if (swf->getBitReader().getStream()->seek(IStream::SeekSet, context.tagEndPosition) < 0)
					going = false;
			}
			else if (uint32_t(swf->getBitReader().getStream()->tell()) > context.tagEndPosition)
			{
				log::error << L"Malformed SWF; read too many bytes (" << swf->getBitReader().getStream()->tell() - context.tagEndPosition << L") in tag " << int32_t(tag->id) << Endl;
				if (swf->getBitReader().getStream()->seek(IStream::SeekSet, context.tagEndPosition) < 0)
					going = false;
			}
		}
		else
		{
			T_DEBUG(L"No reader for tag " << int32_t(tag->id));
			swf->getBitReader().skip(tag->length * 8);
		}

		swf->leaveScope();
	}

#if T_SHOW_STATISTICS
	const std::map< uint16_t, Ref< Character > >& characters = movie->getCharacters();
	std::map< const TypeInfo*, uint32_t > typeCounts;

	// Count number of occurrences of each character type.
	for (std::map< uint16_t, Ref< Character > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		typeCounts[&type_of(i->second)]++;

	log::info << L"Characters loaded:" << Endl;
	for (std::map< const TypeInfo*, uint32_t >::iterator i = typeCounts.begin(); i != typeCounts.end(); ++i)
		log::info << i->first->getName() << L" " << i->second << Endl;
#endif

	return movie;
}

Ref< Movie > MovieFactory::createMovieFromImage(const drawing::Image* image) const
{
	// Create a single frame and place shape.
	Ref< Frame > frame = new Frame();
			
	Frame::PlaceObject p;
	p.hasFlags = Frame::PfHasCharacterId;
	p.depth = 0;
	p.characterId = 1;
	frame->placeObject(p);

	// Create sprite and add frame.
	Ref< Sprite > sprite = new Sprite();
	sprite->addFrame(frame);

	// Create quad shape and fill with bitmap.
	Ref< Shape > shape = new Shape();
	shape->create(1, image->getWidth() * 20, image->getHeight() * 20);

	// Setup dictionary.
	Ref< Movie > movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(image->getWidth() * 20.0f, image->getHeight() * 20.0f)), sprite);
	movie->defineBitmap(1, new BitmapImage(image));
	movie->defineCharacter(1, shape);
	return movie;
}

	}
}
