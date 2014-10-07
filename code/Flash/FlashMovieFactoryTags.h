#ifndef traktor_flash_FlashMovieFactoryTags_H
#define traktor_flash_FlashMovieFactoryTags_H

#include "Core/Object.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class ImageFormatJpeg;

	}

	namespace flash
	{

class SwfReader;
class FlashMovie;
class FlashSprite;
class FlashFrame;

//@{
/*! \ingroup Flash */

class T_DLLCLASS FlashTag : public Object
{
public:
	struct ReadContext
	{
		uint8_t version;
		bool allowNPOT;
		Ref< FlashMovie > movie;
		Ref< FlashSprite > sprite;
		Ref< FlashFrame > frame;
		Ref< drawing::ImageFormatJpeg > jpegFormat;
		uint32_t tagSize;
		uint32_t tagEndPosition;
	};

	virtual bool read(SwfReader* swf, ReadContext& context) = 0;
};

class FlashTagSetBackgroundColor : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDefineShape : public FlashTag
{
public:
	FlashTagDefineShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_shapeType;
};

class FlashTagDefineMorphShape : public FlashTag
{
public:
	FlashTagDefineMorphShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_shapeType;
};

class FlashTagDefineFont : public FlashTag
{
public:
	FlashTagDefineFont(int fontType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_fontType;
};

class FlashTagDefineText : public FlashTag
{
public:
	FlashTagDefineText(int textType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_textType;
};

class FlashTagDefineEditText : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDefineButton : public FlashTag
{
public:
	FlashTagDefineButton(int buttonType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_buttonType;
};

class FlashTagJpegTables : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDefineBitsJpeg : public FlashTag
{
public:
	FlashTagDefineBitsJpeg(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_bitsType;
};

class FlashTagDefineBitsLossLess : public FlashTag
{
public:
	FlashTagDefineBitsLossLess(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_bitsType;
};

class FlashTagDefineSprite : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagPlaceObject : public FlashTag
{
public:
	FlashTagPlaceObject(int placeType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_placeType;
};

class FlashTagRemoveObject : public FlashTag
{
public:
	FlashTagRemoveObject(int removeType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_removeType;
};

class FlashTagShowFrame : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDoAction : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagExportAssets : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagImportAssets : public FlashTag
{
public:
	FlashTagImportAssets(int importType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int m_importType;
};

class FlashTagInitAction : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagProtect : public FlashTag
{
public:
	FlashTagProtect(int protectType);

	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagFrameLabel : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDoABC : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagDefineSound : public FlashTag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context);
};

class FlashTagStartSound : public FlashTag
{
public:
	FlashTagStartSound(int32_t startType);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int32_t m_startType;
};

class FlashTagUnsupported : public FlashTag
{
public:
	FlashTagUnsupported(int32_t tagId);

	virtual bool read(SwfReader* swf, ReadContext& context);

private:
	int32_t m_tagId;
	bool m_visited;
};

//@}

	}
}

#endif	// traktor_flash_FlashMovieFactoryTags_H
