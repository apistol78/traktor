#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieFactoryTags.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfReader.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

#define T_SHOW_STATISTICS 1

namespace traktor
{
	namespace flash
	{

FlashMovieFactory::FlashMovieFactory()
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
	m_tagReaders[TiPlaceObject] = new FlashTagPlaceObject(1);
	m_tagReaders[TiPlaceObject2] = new FlashTagPlaceObject(2);
	m_tagReaders[TiPlaceObject3] = new FlashTagPlaceObject(3);
	m_tagReaders[TiRemoveObject] = new FlashTagRemoveObject(1);
	m_tagReaders[TiRemoveObject2] = new FlashTagRemoveObject(2);
	m_tagReaders[TiDoAction] = new FlashTagDoAction();
	m_tagReaders[TiExportAssets] = new FlashTagExportAssets();
	m_tagReaders[TiImportAssets] = new FlashTagImportAssets(1);
	m_tagReaders[TiImportAssets2] = new FlashTagImportAssets(2);
	m_tagReaders[TiInitAction] = new FlashTagInitAction();
	m_tagReaders[TiShowFrame] = new FlashTagShowFrame();
	m_tagReaders[TiProtect] = new FlashTagProtect(1);
	m_tagReaders[TiEnableDebugger] = new FlashTagProtect(2);
	m_tagReaders[TiEnableDebugger2] = new FlashTagProtect(3);
	m_tagReaders[TiFrameLabel] = new FlashTagFrameLabel();
	m_tagReaders[TiDoABC] = new FlashTagDoABC();

	// Define readers for tags which isn't planed to be implemented.
	m_tagReaders[TiDefineFontInfo] = new FlashTagUnsupported(TiDefineFontInfo);
	m_tagReaders[TiDefineSound] = new FlashTagUnsupported(TiDefineSound);
	m_tagReaders[TiStartSound] = new FlashTagUnsupported(TiStartSound);
	m_tagReaders[TiSoundStreamHead] = new FlashTagUnsupported(TiSoundStreamHead);
	m_tagReaders[TiSoundStreamBlock] = new FlashTagUnsupported(TiSoundStreamBlock);
	m_tagReaders[TiSoundStreamHead2] = new FlashTagUnsupported(TiSoundStreamHead2);
	m_tagReaders[TiFileAttributes] = new FlashTagUnsupported(TiFileAttributes);
	m_tagReaders[TiMetadata] = new FlashTagUnsupported(TiMetadata);
	m_tagReaders[TiDefineFontAlignZones] = new FlashTagUnsupported(TiDefineFontAlignZones);
	m_tagReaders[TiCSMTextSettings] = new FlashTagUnsupported(TiCSMTextSettings);
	m_tagReaders[TiDefineFontName] = new FlashTagUnsupported(TiDefineFontName);
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
	Ref< FlashSprite > movieClip = new FlashSprite(0, header->frameRate >> 8);
	Ref< FlashMovie > movie = new FlashMovie(header->frameRect, movieClip);

	// Decode tags.
	FlashTag::ReadContext context;
	context.version = header->version;
	context.movie = movie;
	context.sprite = movieClip;
	context.frame = new FlashFrame();
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
				swf->getBitReader().getStream()->seek(IStream::SeekSet, context.tagEndPosition);
			}
			else if (uint32_t(swf->getBitReader().getStream()->tell()) > context.tagEndPosition)
			{
				log::error << L"Read too many bytes (" << swf->getBitReader().getStream()->tell() - context.tagEndPosition << L") in tag " << int32_t(tag->id) << Endl;
				swf->getBitReader().getStream()->seek(IStream::SeekSet, context.tagEndPosition);
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

	}
}
