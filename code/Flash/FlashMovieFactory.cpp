#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieFactoryTags.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/Avm1/ActionVM1.h"
#include "Flash/Action/Avm2/ActionVM2.h"

#define T_SHOW_STATISTICS 0

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieFactory", FlashMovieFactory, Object)

FlashMovieFactory::FlashMovieFactory(bool includeAS)
:	m_includeAS(includeAS)
{
	// Setup tag readers.
	m_tagReaders[TiSetBackgroundColor] = new FlashTagSetBackgroundColor();
	m_tagReaders[TiDefineShape] = new FlashTagDefineShape(1);
	m_tagReaders[TiDefineShape2] = new FlashTagDefineShape(2);
	m_tagReaders[TiDefineShape3] = new FlashTagDefineShape(3);
	m_tagReaders[TiDefineShape4] = new FlashTagDefineShape(4);
	m_tagReaders[TiDefineMorphShape] = new FlashTagDefineMorphShape(1);
	m_tagReaders[TiDefineMorphShape2] = new FlashTagDefineMorphShape(2);
	m_tagReaders[TiDefineFont] = new FlashTagDefineFont(1);
	m_tagReaders[TiDefineFont2] = new FlashTagDefineFont(2);
	m_tagReaders[TiDefineFont3] = new FlashTagDefineFont(3);
	m_tagReaders[TiDefineScalingGrid] = new FlashTagDefineScalingGrid();
	m_tagReaders[TiJpegTables] = new FlashTagJpegTables();
	m_tagReaders[TiDefineBitsJpeg] = new FlashTagDefineBitsJpeg(1);
	m_tagReaders[TiDefineBitsJpeg2] = new FlashTagDefineBitsJpeg(2);
	m_tagReaders[TiDefineBitsJpeg3] = new FlashTagDefineBitsJpeg(3);
	m_tagReaders[TiDefineBitsLossLess] = new FlashTagDefineBitsLossLess(1);
	m_tagReaders[TiDefineBitsLossLess2] = new FlashTagDefineBitsLossLess(2);
	m_tagReaders[TiDefineSprite] = new FlashTagDefineSprite();
	m_tagReaders[TiDefineText] = new FlashTagDefineText(1);
	m_tagReaders[TiDefineText2] = new FlashTagDefineText(2);
	m_tagReaders[TiDefineEditText] = new FlashTagDefineEditText();
	//m_tagReaders[TiDefineButton] = new FlashTagDefineButton(1);
	m_tagReaders[TiDefineButton2] = new FlashTagDefineButton(2);
	m_tagReaders[TiDefineSceneAndFrameLabelData] = new FlashTagDefineSceneAndFrameLabelData();
	m_tagReaders[TiPlaceObject] = new FlashTagPlaceObject(1);
	m_tagReaders[TiPlaceObject2] = new FlashTagPlaceObject(2);
	m_tagReaders[TiPlaceObject3] = new FlashTagPlaceObject(3);
	m_tagReaders[TiRemoveObject] = new FlashTagRemoveObject(1);
	m_tagReaders[TiRemoveObject2] = new FlashTagRemoveObject(2);
	m_tagReaders[TiDoAction] = new FlashTagDoAction();
	m_tagReaders[TiExportAssets] = new FlashTagExportAssets();
	m_tagReaders[TiImportAssets] = new FlashTagImportAssets(1);
	m_tagReaders[TiImportAssets2] = new FlashTagImportAssets(2);
	m_tagReaders[TiSymbolClass] = new FlashTagSymbolClass();
	m_tagReaders[TiMetadata] = new FlashTagMetaData();
	
	if (m_includeAS)
		m_tagReaders[TiInitAction] = new FlashTagInitAction();

	m_tagReaders[TiShowFrame] = new FlashTagShowFrame();
	m_tagReaders[TiProtect] = new FlashTagProtect(1);
	m_tagReaders[TiEnableDebugger] = new FlashTagProtect(2);
	m_tagReaders[TiEnableDebugger2] = new FlashTagProtect(3);
	m_tagReaders[TiFrameLabel] = new FlashTagFrameLabel();
	
	if (m_includeAS)
		m_tagReaders[TiDoABC] = new FlashTagDoABC();

	m_tagReaders[TiDefineSound] = new FlashTagDefineSound();
	m_tagReaders[TiStartSound] = new FlashTagStartSound(1);
	m_tagReaders[TiStartSound2] = new FlashTagStartSound(2);

	// Define readers for tags which isn't planed to be implemented.
	m_tagReaders[TiDefineFontInfo] = new FlashTagUnsupported(TiDefineFontInfo);
	m_tagReaders[TiSoundStreamHead] = new FlashTagUnsupported(TiSoundStreamHead);
	m_tagReaders[TiSoundStreamBlock] = new FlashTagUnsupported(TiSoundStreamBlock);
	m_tagReaders[TiSoundStreamHead2] = new FlashTagUnsupported(TiSoundStreamHead2);
	m_tagReaders[TiDebugID] = new FlashTagUnsupported(TiDebugID);
	m_tagReaders[TiFileAttributes] = new FlashTagUnsupported(TiFileAttributes);
	m_tagReaders[TiDefineFontAlignZones] = new FlashTagUnsupported(TiDefineFontAlignZones);
	m_tagReaders[TiCSMTextSettings] = new FlashTagUnsupported(TiCSMTextSettings);
	m_tagReaders[TiDefineFontName] = new FlashTagUnsupported(TiDefineFontName);
}

Ref< FlashMovie > FlashMovieFactory::createMovie(SwfReader* swf) const
{
	SwfHeader* header = swf->readHeader();
	if (!header)
	{
		log::error << L"Unable to read SWF movie; invalid header" << Endl;
		return 0;
	}

	T_DEBUG(L"SWF movie version " << int32_t(header->version));

	// Create new movie.
	Ref< FlashSprite > movieClip = new FlashSprite(0, header->frameRate >> 8);
	Ref< FlashMovie > movie = new FlashMovie(header->frameRect, movieClip);

	// Decode tags.
	FlashTag::ReadContext context;
	context.version = header->version;
	context.movie = movie;
	context.sprite = movieClip;
	context.frame = new FlashFrame();

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

		Ref< FlashTag > tagReader = m_tagReaders.at(tag->id);
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
	const std::map< uint16_t, Ref< FlashCharacter > >& characters = movie->getCharacters();
	std::map< const TypeInfo*, uint32_t > typeCounts;

	// Count number of occurrences of each character type.
	for (std::map< uint16_t, Ref< FlashCharacter > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		typeCounts[&type_of(i->second)]++;

	log::info << L"Characters loaded:" << Endl;
	for (std::map< const TypeInfo*, uint32_t >::iterator i = typeCounts.begin(); i != typeCounts.end(); ++i)
		log::info << i->first->getName() << L" " << i->second << Endl;
#endif

	return movie;
}

Ref< FlashMovie > FlashMovieFactory::createMovieFromImage(const drawing::Image* image) const
{
	// Create a single frame and place shape.
	Ref< FlashFrame > frame = new FlashFrame();
			
	FlashFrame::PlaceObject p;
	p.hasFlags = FlashFrame::PfHasCharacterId;
	p.depth = 0;
	p.characterId = 1;
	frame->placeObject(p);

	// Create sprite and add frame.
	Ref< FlashSprite > sprite = new FlashSprite();
	sprite->addFrame(frame);

	// Create quad shape and fill with bitmap.
	Ref< FlashShape > shape = new FlashShape();
	shape->create(1, image->getWidth() * 20, image->getHeight() * 20);

	// Setup dictionary.
	Ref< FlashMovie > movie = new FlashMovie(Aabb2(Vector2(0.0f, 0.0f), Vector2(image->getWidth() * 20, image->getHeight() * 20)), sprite);
	movie->defineBitmap(1, new FlashBitmapImage(image));
	movie->defineCharacter(1, shape);
	return movie;
}

	}
}
