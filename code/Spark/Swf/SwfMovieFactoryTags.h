/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Spark/Swf/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class ImageFormatJpeg;

}

namespace traktor::spark
{

class SwfReader;
class Movie;
class Sprite;
class Frame;

//@{
/*! \ingroup Spark */

class T_DLLCLASS Tag : public Object
{
public:
	struct ReadContext
	{
		uint8_t version;
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
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagDefineShape : public Tag
{
public:
	TagDefineShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_shapeType;
};

class TagDefineMorphShape : public Tag
{
public:
	TagDefineMorphShape(int shapeType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_shapeType;
};

class TagDefineFont : public Tag
{
public:
	TagDefineFont(int fontType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_fontType;
};

class TagDefineScalingGrid : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagDefineText : public Tag
{
public:
	TagDefineText(int textType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_textType;
};

class TagDefineEditText : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagDefineButton : public Tag
{
public:
	TagDefineButton(int buttonType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_buttonType;
};

class TagJpegTables : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagDefineBitsJpeg : public Tag
{
public:
	TagDefineBitsJpeg(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_bitsType;
};

class TagDefineBitsLossLess : public Tag
{
public:
	TagDefineBitsLossLess(int bitsType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_bitsType;
};

class TagDefineSprite : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagPlaceObject : public Tag
{
public:
	TagPlaceObject(int placeType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_placeType;
};

class TagRemoveObject : public Tag
{
public:
	TagRemoveObject(int removeType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_removeType;
};

class TagShowFrame : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

//class TagDoAction : public Tag
//{
//public:
//	virtual bool read(SwfReader* swf, ReadContext& context) override final;
//};

class TagExportAssets : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagImportAssets : public Tag
{
public:
	TagImportAssets(int importType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int m_importType;
};

//class TagInitAction : public Tag
//{
//public:
//	virtual bool read(SwfReader* swf, ReadContext& context) override final;
//};

class TagProtect : public Tag
{
public:
	TagProtect(int protectType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagFrameLabel : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

//class TagDoABC : public Tag
//{
//public:
//	virtual bool read(SwfReader* swf, ReadContext& context) override final;
//};

class TagDefineSound : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagStartSound : public Tag
{
public:
	TagStartSound(int32_t startType);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int32_t m_startType;
};

class TagDefineSceneAndFrameLabelData : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagSymbolClass : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagMetaData : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagFileAttributes : public Tag
{
public:
	virtual bool read(SwfReader* swf, ReadContext& context) override final;
};

class TagUnsupported : public Tag
{
public:
	TagUnsupported(int32_t tagId);

	virtual bool read(SwfReader* swf, ReadContext& context) override final;

private:
	int32_t m_tagId;
	bool m_visited;
};

//@}

}
