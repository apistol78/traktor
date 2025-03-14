/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor::spark
{

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

//@{
/*! \ingroup Spark */

struct SwfMatrix
{
	float m[9];
};

struct SwfKerningRecord
{
	uint16_t code1;
	uint16_t code2;
	int16_t adjustment;
};

struct SwfHeader
{
	uint8_t signature[3];
	uint8_t version;
	uint32_t fileSize;
	Aabb2 frameRect;
	uint16_t frameRate;
	uint16_t frameCount;
};

enum SwfTagIds
{
	TiEnd							= 0,
	TiShowFrame						= 1,
	TiDefineShape					= 2,
	TiPlaceObject					= 4,
	TiRemoveObject					= 5,
	TiDefineBitsJpeg				= 6,
	TiDefineButton					= 7,
	TiJpegTables					= 8,
	TiSetBackgroundColor			= 9,
	TiDefineFont					= 10,
	TiDefineText					= 11,
	TiDoAction						= 12,
	TiDefineFontInfo				= 13,
	TiDefineSound					= 14,
	TiStartSound					= 15,
	TiDefineButtonSound				= 17,
	TiSoundStreamHead				= 18,
	TiSoundStreamBlock				= 19,
	TiDefineBitsLossLess			= 20,
	TiDefineBitsJpeg2				= 21,
	TiDefineShape2					= 22,
	TiDefineButtonCxform			= 23,
	TiProtect						= 24,
	TiPlaceObject2					= 26,
	TiRemoveObject2					= 28,
	TiDefineShape3					= 32,
	TiDefineText2					= 33,
	TiDefineButton2					= 34,
	TiDefineBitsJpeg3				= 35,
	TiDefineBitsLossLess2			= 36,
	TiDefineEditText				= 37,
	TiDefineSprite					= 39,
	TiFrameLabel					= 43,
	TiSoundStreamHead2				= 45,
	TiDefineMorphShape				= 46,
	TiDefineFont2					= 48,
	TiExportAssets					= 56,
	TiImportAssets					= 57,
	TiEnableDebugger				= 58,
	TiInitAction					= 59,
	TiDefineVideoStream				= 60,
	TiVideoFrame					= 61,
	TiDefineFontInfo2				= 62,
	TiDebugID						= 63,
	TiEnableDebugger2				= 64,
	TiScriptLimits					= 65,
	TiSetTabIndex					= 66,
	TiFileAttributes				= 69,
	TiPlaceObject3					= 70,
	TiImportAssets2					= 71,
	TiDefineFontAlignZones			= 73,
	TiCSMTextSettings				= 74,
	TiDefineFont3					= 75,
	TiSymbolClass					= 76,
	TiMetadata						= 77,
	TiDefineScalingGrid				= 78,
	TiDoABC							= 82,
	TiDefineShape4					= 83,
	TiDefineMorphShape2				= 84,
	TiDefineSceneAndFrameLabelData	= 86,
	TiDefineBinaryData				= 87,
	TiDefineFontName				= 88,
	TiStartSound2					= 89,
	TiDefineBitsJPEG4				= 90,
	TiDefineFont4					= 91
};

struct SwfTag
{
	uint8_t id;
	uint32_t length;
};

struct SwfGradientRecord
{
	uint8_t ratio;
	Color4f color;
};

struct SwfGradient
{
	uint8_t numGradientRecords;
	SwfGradientRecord** gradientRecords;
	float focalPoint;
};

enum SwfFillStyleTypes
{
	FstSolid				= 0x00,
	FstLinearGradient		= 0x10,
	FstRadialGradient		= 0x12,
	FstFocalGradient		= 0x13,
	FstTiledBitmap			= 0x40,
	FstClippedBitmap		= 0x41,
	FstTiledBitmapHard		= 0x42,
	FstClippedBitmapHard	= 0x43
};

struct SwfFillStyleSolid
{
	float color[4];
};

struct SwfFillStyleGradient
{
	SwfMatrix gradientMatrix;
	SwfGradient* gradient;
};

struct SwfFillStyleBitmap
{
	uint16_t bitmapId;
	SwfMatrix bitmapMatrix;
};

struct SwfFillStyle
{
	uint8_t type;
	union
	{
		SwfFillStyleSolid solid;
		SwfFillStyleGradient gradient;
		SwfFillStyleBitmap bitmap;
	};
};

struct SwfLineStyle
{
	uint16_t width;
	Color4f color;
};

struct SwfStyles
{
	uint16_t numFillStyles;
	SwfFillStyle** fillStyles;
	uint16_t numLineStyles;
	SwfLineStyle** lineStyles;
	uint8_t numFillBits;
	uint8_t numLineBits;
};

struct SwfStyleRecord
{
	bool stateNewStyles;
	bool stateLineStyle;
	bool stateFillStyle0;
	bool stateFillStyle1;
	bool stateMoveTo;
	int32_t moveDeltaX;
	int32_t moveDeltaY;
	uint32_t fillStyle0;
	uint32_t fillStyle1;
	uint32_t lineStyle;
	SwfStyles* newStyles;
};

struct SwfStraightEdgeRecord
{
	bool generalLineFlag;
	bool vertLineFlag;
	int32_t deltaX;
	int32_t deltaY;
};

struct SwfCurvedEdgeRecord
{
	int32_t controlDeltaX;
	int32_t controlDeltaY;
	int32_t anchorDeltaX;
	int32_t anchorDeltaY;
};

struct SwfEdgeRecord
{
	bool straightFlag;
	union
	{
		SwfStraightEdgeRecord straightEdge;
		SwfCurvedEdgeRecord curvedEdge;
	};
};

struct SwfShapeRecord
{
	bool edgeFlag;
	union
	{
		SwfStyleRecord style;
		SwfEdgeRecord edge;
	};
	SwfShapeRecord* next;
};

struct SwfShape
{
	SwfShapeRecord* shapeRecords;
};

struct SwfTextStyleRecord
{
	bool hasFont;
	bool hasColor;
	bool hasYOffset;
	bool hasXOffset;
	uint16_t fontId;
	float color[4];
	int16_t XOffset;
	int16_t YOffset;
	int16_t height;
};

struct SwfGlyphEntry
{
	uint32_t glyphIndex;
	int32_t glyphAdvance;
};

struct SwfTextGlyphRecord
{
	uint8_t glyphCount;
	SwfGlyphEntry** glyphEntries;
};

struct SwfTextRecord
{
	bool styleFlag;
	union
	{
		SwfTextStyleRecord style;
		SwfTextGlyphRecord glyph;
	};
};

struct SwfFilterDropShadow
{
	float dropShadowColor[4];
	float blurX;
	float blurY;
	float angle;
	float distance;
	float strength;
	bool innerShadow;
	bool knockOut;
	bool compositeSource;
	uint8_t passes;
};

struct SwfFilterBlur
{
	float blurX;
	float blurY;
	uint8_t passes;
};

struct SwfFilterGlow
{
	float glowColor[4];
	float blurX;
	float blurY;
	float strength;
	bool innerGlow;
	bool knockOut;
	bool compositeSource;
	uint8_t passes;
};

struct SwfFilterBevel
{
	float shadowColor[4];
	float highlightColor[4];
	float blurX;
	float blurY;
	float angle;
	float distance;
	float strength;
	bool innerShadow;
	bool knockOut;
	bool compositeSource;
	bool onTop;
	uint8_t passes;
};

struct SwfFilterGradientGlow
{
	uint8_t numColors;
	float gradientColors[4];
	uint8_t* gradientRatio;
	float blurX;
	float blurY;
	float distance;
	float strength;
	bool innerShadow;
	bool knockOut;
	bool compositeSource;
	bool onTop;
	uint8_t passes;
};

struct SwfFilterConvolution
{
	uint8_t matrixX;
	uint8_t matrixY;
	float divisor;
	float bias;
	float* matrix;
	float defaultColor[4];
	bool clamp;
	bool preserveAlpha;
};

struct SwfFilterColorMatrix
{
	float matrix[20];
};

struct SwfFilterGradientBevel
{
	uint8_t numColors;
	float gradientColors[4];
	uint8_t* gradientRatio;
	float blurX;
	float blurY;
	float distance;
	float strength;
	bool innerShadow;
	bool knockOut;
	bool compositeSource;
	bool onTop;
	uint8_t passes;
};

struct SwfFilter
{
	uint8_t filterId;
	union
	{
		SwfFilterDropShadow dropShadow;			// 0
		SwfFilterBlur blur;						// 1
		SwfFilterGlow glow;						// 2
		SwfFilterBevel bevel;					// 3
		SwfFilterGradientGlow gradientGlow;		// 4
		SwfFilterConvolution convolution;		// 5
		SwfFilterColorMatrix colorMatrix;		// 6
		SwfFilterGradientBevel gradientBevel;	// 7
	};
};

struct SwfSoundInfo
{
	bool syncStop;
	bool syncNoMultiple;
};

enum SwfEventType
{
	EvtMouseWheel		= (1 << 21),
	EvtKey				= (1 << 20),
	EvtViewResize		= (1 << 19),
	EvtConstruct		= (1 << 18),
	EvtKeyPress			= (1 << 17),
	EvtDragOut			= (1 << 16),
	EvtDragOver			= (1 << 15),
	EvtRollOut			= (1 << 14),
	EvtRollOver			= (1 << 13),
	EvtReleaseOutside	= (1 << 12),
	EvtRelease			= (1 << 11),
	EvtPress			= (1 << 10),
	EvtInitialize		= (1 << 9),
	EvtData				= (1 << 8),
	EvtKeyUp			= (1 << 7),
	EvtKeyDown			= (1 << 6),
	EvtMouseUp			= (1 << 5),
	EvtMouseDown		= (1 << 4),
	EvtMouseMove		= (1 << 3),
	EvtUnload			= (1 << 2),
	EvtEnterFrame		= (1 << 1),
	EvtLoad				= (1 << 0)
};

enum SwfAlignType
{
	SaTop = 0,
	SaLeft = 0,
	SaCenter = 1,
	SaBottom = 2,
	SaRight = 2
};

enum SwfTextAlignType
{
	StaLeft = 0,
	StaRight = 1,
	StaCenter = 2,
	StaJustify = 3
};

enum SwfBlendMode
{
	SbmDefault = 0,
	SbmNormal = 1,
	SbmLayer = 2,
	SbmMultiply = 3,
	SbmScreen = 4,
	SbmLighten = 5,
	SbmDarken = 6,
	SbmDifference = 7,
	SbmAdd = 8,
	SbmSubtract = 9,
	SbmInvert = 10,
	SbmAlpha = 11,
	SbmErase = 12,
	SbmOverlay = 13,
	SbmHardlight = 14,
	SbmOpaque = 15,		//!< Not actually part of SWF.
	SbmLast
};

//@}

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

}
