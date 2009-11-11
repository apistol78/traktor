#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieFactoryTags.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfReader.h"
#include "Core/Heap/GcNew.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

#define T_SHOW_STATISTICS 1

namespace traktor
{
	namespace flash
	{

FlashMovieFactory::FlashMovieFactory()
{
	// Setup tag readers.
	m_tagReaders[TiSetBackgroundColor] = gc_new< FlashTagSetBackgroundColor >();
	m_tagReaders[TiDefineShape] = gc_new< FlashTagDefineShape >(1);
	m_tagReaders[TiDefineShape2] = gc_new< FlashTagDefineShape >(2);
	m_tagReaders[TiDefineShape3] = gc_new< FlashTagDefineShape >(3);
	m_tagReaders[TiDefineShape4] = gc_new< FlashTagDefineShape >(4);
	m_tagReaders[TiDefineMorphShape] = gc_new< FlashTagDefineMorphShape >(1);
	m_tagReaders[TiDefineMorphShape2] = gc_new< FlashTagDefineMorphShape >(2);
	m_tagReaders[TiDefineFont] = gc_new< FlashTagDefineFont >(1);
	m_tagReaders[TiDefineFont2] = gc_new< FlashTagDefineFont >(2);
	m_tagReaders[TiDefineFont3] = gc_new< FlashTagDefineFont >(3);
	m_tagReaders[TiJpegTables] = gc_new< FlashTagJpegTables >();
	m_tagReaders[TiDefineBitsJpeg] = gc_new< FlashTagDefineBitsJpeg >(1);
	m_tagReaders[TiDefineBitsJpeg2] = gc_new< FlashTagDefineBitsJpeg >(2);
	m_tagReaders[TiDefineBitsJpeg3] = gc_new< FlashTagDefineBitsJpeg >(3);
	m_tagReaders[TiDefineBitsLossLess] = gc_new< FlashTagDefineBitsLossLess >(1);
	m_tagReaders[TiDefineBitsLossLess2] = gc_new< FlashTagDefineBitsLossLess >(2);
	m_tagReaders[TiDefineSprite] = gc_new< FlashTagDefineSprite >();
	m_tagReaders[TiDefineText] = gc_new< FlashTagDefineText >(1);
	m_tagReaders[TiDefineText2] = gc_new< FlashTagDefineText >(2);
	m_tagReaders[TiDefineEditText] = gc_new< FlashTagDefineEditText >();
	//m_tagReaders[TiDefineButton] = gc_new< FlashTagDefineButton >(1);
	m_tagReaders[TiDefineButton2] = gc_new< FlashTagDefineButton >(2);
	m_tagReaders[TiPlaceObject] = gc_new< FlashTagPlaceObject >(1);
	m_tagReaders[TiPlaceObject2] = gc_new< FlashTagPlaceObject >(2);
	m_tagReaders[TiPlaceObject3] = gc_new< FlashTagPlaceObject >(3);
	m_tagReaders[TiRemoveObject] = gc_new< FlashTagRemoveObject >(1);
	m_tagReaders[TiRemoveObject2] = gc_new< FlashTagRemoveObject >(2);
	m_tagReaders[TiDoAction] = gc_new< FlashTagDoAction >();
	m_tagReaders[TiExportAssets] = gc_new< FlashTagExportAssets >();
	m_tagReaders[TiImportAssets] = gc_new< FlashTagImportAssets >(1);
	m_tagReaders[TiImportAssets2] = gc_new< FlashTagImportAssets >(2);
	m_tagReaders[TiInitAction] = gc_new< FlashTagInitAction >();
	m_tagReaders[TiShowFrame] = gc_new< FlashTagShowFrame >();
	m_tagReaders[TiProtect] = gc_new< FlashTagProtect >(1);
	m_tagReaders[TiEnableDebugger] = gc_new< FlashTagProtect >(2);
	m_tagReaders[TiEnableDebugger2] = gc_new< FlashTagProtect >(3);
	m_tagReaders[TiFrameLabel] = gc_new< FlashTagFrameLabel >();

	// Define readers for tags which isn't planed to be implemented.
	m_tagReaders[TiDefineFontInfo] = gc_new< FlashTagUnsupported >(TiDefineFontInfo);
	m_tagReaders[TiDefineSound] = gc_new< FlashTagUnsupported >(TiDefineSound);
	m_tagReaders[TiStartSound] = gc_new< FlashTagUnsupported >(TiStartSound);
	m_tagReaders[TiSoundStreamHead] = gc_new< FlashTagUnsupported >(TiSoundStreamHead);
	m_tagReaders[TiSoundStreamBlock] = gc_new< FlashTagUnsupported >(TiSoundStreamBlock);
	m_tagReaders[TiSoundStreamHead2] = gc_new< FlashTagUnsupported >(TiSoundStreamHead2);
	m_tagReaders[TiFileAttributes] = gc_new< FlashTagUnsupported >(TiFileAttributes);
	m_tagReaders[TiMetadata] = gc_new< FlashTagUnsupported >(TiMetadata);
	m_tagReaders[TiDefineFontAlignZones] = gc_new< FlashTagUnsupported >(TiDefineFontAlignZones);
	m_tagReaders[TiCSMTextSettings] = gc_new< FlashTagUnsupported >(TiCSMTextSettings);
	m_tagReaders[TiDefineFontName] = gc_new< FlashTagUnsupported >(TiDefineFontName);
}

Ref< FlashMovie > FlashMovieFactory::createMovie(SwfReader* swf)
{
	SwfHeader* header = swf->readHeader();
	if (!header)
	{
		log::error << L"Unable to read Flash movie, invalid header" << Endl;
		return 0;
	}

	log::info << L"Flash movie version " << int32_t(header->version) << Endl;

	// Create new movie.
	Ref< FlashSprite > movieClip = gc_new< FlashSprite >(0, header->frameRate >> 8);
	Ref< FlashMovie > movie = gc_new< FlashMovie >(cref(header->frameRect), movieClip);

	// Decode tags.
	FlashTag::ReadContext context;
	context.version = header->version;
	context.movie = movie;
	context.sprite = movieClip;
	context.frame = gc_new< FlashFrame >();
	for (;;)
	{
		swf->enterScope();

		SwfTag* tag = swf->readTag();
		if (!tag || tag->id == TiEnd)
			break;

		Ref< FlashTag > tagReader = m_tagReaders[tag->id];
		if (tagReader)
		{
			context.tagSize = tag->length;
			context.tagEndPosition = swf->getBitReader().getStream()->tell() + tag->length;
			if (!tagReader->read(swf, context))
			{
				log::error << L"Unable to read flash, error when reading tag " << int32_t(tag->id) << Endl;
				return 0;
			}
			if (uint32_t(swf->getBitReader().getStream()->tell()) < context.tagEndPosition)
			{
				log::warning << L"Read too few bytes (" << context.tagEndPosition - swf->getBitReader().getStream()->tell() << L" left) in tag " << int32_t(tag->id) << Endl;
				swf->getBitReader().getStream()->seek(Stream::SeekSet, context.tagEndPosition);
			}
			else if (uint32_t(swf->getBitReader().getStream()->tell()) > context.tagEndPosition)
			{
				log::error << L"Read too many bytes (" << swf->getBitReader().getStream()->tell() - context.tagEndPosition << L") in tag " << int32_t(tag->id) << Endl;
				swf->getBitReader().getStream()->seek(Stream::SeekSet, context.tagEndPosition);
			}
		}
		else
		{
			log::warning << L"No reader for tag " << int32_t(tag->id) << Endl;
			swf->getBitReader().skip(tag->length * 8);
		}

		swf->leaveScope();
	}

#if T_SHOW_STATISTICS
	const std::map< uint16_t, Ref< FlashCharacter > >& characters = movie->getCharacters();
	std::map< const Type*, uint32_t > typeCounts;

	// Count number of occurrences of each character type.
	for (std::map< uint16_t, Ref< FlashCharacter > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		typeCounts[&i->second->getType()]++;

	log::info << L"Characters loaded:" << Endl;
	for (std::map< const Type*, uint32_t >::iterator i = typeCounts.begin(); i != typeCounts.end(); ++i)
		log::info << i->first->getName() << L" " << i->second << Endl;
#endif

	return movie;
}

	}
}
