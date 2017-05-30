/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MovieFactoryTags_H
#define traktor_flash_MovieFactoryTags_H

#include "Core/Object.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class ImageFormatJpeg;

	}

	namespace flash
	{

class IActionVM;
class SwfReader;
class Movie;
class Sprite;
class Frame;

//@{
/*! \ingroup Flash */

class T_DLLCLASS Tag : public Object
{
public:
	struct ReadContext
	{
		uint8_t version;
		Ref< IActionVM > avm1;
		Ref< IActionVM > avm2;
		Ref< Movie > movie;
		Ref< Sprite > sprite;
		Ref< Frame > frame;
		Ref< drawing::ImageFormatJpeg > jpegFormat;
		uint32_t tagSize;
		int64_t tagEndPosition;
	};

	virtual bool read(SwfReader* swf, ReadContext& context) = 0;
};

class TagSetBackgroundColor : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDefineShape : public Tag
{
public:
	TagDefineShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_shapeType;
};

class TagDefineMorphShape : public Tag
{
public:
	TagDefineMorphShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_shapeType;
};

class TagDefineFont : public Tag
{
public:
	TagDefineFont(int fontType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_fontType;
};

class TagDefineScalingGrid : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDefineText : public Tag
{
public:
	TagDefineText(int textType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_textType;
};

class TagDefineEditText : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDefineButton : public Tag
{
public:
	TagDefineButton(int buttonType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_buttonType;
};

class TagJpegTables : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDefineBitsJpeg : public Tag
{
public:
	TagDefineBitsJpeg(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_bitsType;
};

class TagDefineBitsLossLess : public Tag
{
public:
	TagDefineBitsLossLess(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_bitsType;
};

class TagDefineSprite : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagPlaceObject : public Tag
{
public:
	TagPlaceObject(int placeType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_placeType;
};

class TagRemoveObject : public Tag
{
public:
	TagRemoveObject(int removeType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_removeType;
};

class TagShowFrame : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDoAction : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagExportAssets : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagImportAssets : public Tag
{
public:
	TagImportAssets(int importType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int m_importType;
};

class TagInitAction : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagProtect : public Tag
{
public:
	TagProtect(int protectType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagFrameLabel : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDoABC : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagDefineSound : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagStartSound : public Tag
{
public:
	TagStartSound(int32_t startType);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int32_t m_startType;
};

class TagDefineSceneAndFrameLabelData : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagSymbolClass : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagMetaData : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagFileAttributes : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;
};

class TagUnsupported : public Tag
{
public:
	TagUnsupported(int32_t tagId);

	virtual bool read(SwfReader* swf, ReadContext& context) T_OVERRIDE T_FINAL;

private:
	int32_t m_tagId;
	bool m_visited;
};

//@}

	}
}

#endif	// traktor_flash_MovieFactoryTags_H
