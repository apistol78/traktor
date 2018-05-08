/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_LwBlock_H
#define traktor_model_LwBlock_H

#include <string>
#include "Core/Object.h"
#include "Core/Math/Vector4.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk;

class LwBlock : public Object
{
	T_RTTI_CLASS;

public:
	LwBlock();

	bool read(const LwChunk& chunk, IStream* stream);

private:
	uint32_t m_type;			// Block type : IMAP, PROC, GRAD, SHDR
	std::wstring m_ordinal;
	uint32_t m_channel;			// COLR, DIFF, LUMI, SPEC, GLOS, REFL, TRAN, RIND, TRNL or BUMP
	bool m_enable;
	uint16_t m_opacityType;
	float m_opacity;
	uint32_t m_opacityEnvelope;
	uint16_t m_displacementAxis;
	
	Vector4 m_textureCenter;	// IMAP or PROC only
	uint32_t m_textureCenterEnvelope;
	Vector4 m_textureSize;
	uint32_t m_textureSizeEnvelope;
	Vector4 m_textureRotate;
	uint32_t m_textureRotateEnvelope;
	std::wstring m_textureReferenceObject;
	uint16_t m_textureFallOffType;
	Vector4 m_textureFallOffVector;
	uint32_t m_textureFallOffEnvelope;
	uint16_t m_textureCoordinateSystem;
	
	uint32_t m_textureImage;		// IMAP only
	uint16_t m_textureWidthWrap;
	uint16_t m_textureHeightWrap;
	std::wstring m_textureVertexMap;
};

	}
}

#endif	// traktor_model_LwBlock_H
