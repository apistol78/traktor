/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Model/Formats/LwBlock.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwRead.h"
#include "Model/Formats/LwTags.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwBlock", LwBlock, Object)

LwBlock::LwBlock()
:	m_type(0)
,	m_channel(0)
,	m_enable(false)
,	m_opacityType(0)
,	m_opacity(0)
,	m_opacityEnvelope(0)
,	m_displacementAxis(0)
,	m_textureCenterEnvelope(0)
,	m_textureSizeEnvelope(0)
,	m_textureRotateEnvelope(0)
,	m_textureFallOffType(0)
,	m_textureFallOffEnvelope(0)
,	m_textureCoordinateSystem(0)
,	m_textureImage(0)
,	m_textureWidthWrap(0)
,	m_textureHeightWrap(0)
{
}

bool LwBlock::read(const LwChunk& chunk, IStream* stream)
{
	LwChunk sub;

	if (!sub.readSub(stream))
		return false;

	m_type = sub.tag;
	
	if (!lwRead< std::wstring >(stream, m_ordinal))
		return false;
		
	while (stream->tell() < chunk.end)
	{
		if (!sub.readSub(stream))
			return false;
			
		if (sub.tag == LwTags::CHAN)
		{
			if (!lwRead< uint32_t >(stream, m_channel))
				return false;
		}
		else if (sub.tag == LwTags::ENAB)
		{
			uint16_t enable;
			if (!lwRead< uint16_t >(stream, enable))
				return false;
			m_enable = enable > 0;
		}
		else if (sub.tag == LwTags::OPAC)
		{
			if (!lwRead< uint16_t >(stream, m_opacityType))
				return false;
			if (!lwRead< float >(stream, m_opacity))
				return false;
			if (!lwReadIndex(stream, m_opacityEnvelope))
				return false;
		}
		else if (sub.tag == LwTags::AXIS)
		{
			if (!lwRead< uint16_t >(stream, m_displacementAxis))
				return false;
		}
		else if (sub.tag == LwTags::TMAP)
		{
			while (stream->tell() < sub.end)
			{
				LwChunk tsub;

				if (!tsub.readSub(stream))
					return false;
					
				if (tsub.tag == LwTags::CNTR)
				{
					if (!lwRead< Vector4 >(stream, m_textureCenter))
						return false;

					if (!lwReadIndex(stream, m_textureCenterEnvelope))
						return false;
				}
				else if (tsub.tag == LwTags::SIZE)
				{
					if (!lwRead< Vector4 >(stream, m_textureSize))
						return false;

					if (!lwReadIndex(stream, m_textureSizeEnvelope))
						return false;
				}
				else if (tsub.tag == LwTags::ROTA)
				{
					if (!lwRead< Vector4 >(stream, m_textureRotate))
						return false;

					if (!lwReadIndex(stream, m_textureRotateEnvelope))
						return false;
				}
				else if (tsub.tag == LwTags::OREF)
				{
					if (!lwRead< std::wstring >(stream, m_textureReferenceObject))
						return false;
				}
				else if (tsub.tag == LwTags::FALL)
				{
					if (!lwRead< uint16_t >(stream, m_textureFallOffType))
						return false;

					if (!lwRead< Vector4 >(stream, m_textureFallOffVector))
						return false;

					if (!lwReadIndex(stream, m_textureFallOffEnvelope))
						return false;
				}
				else if (tsub.tag == LwTags::CSYS)
				{
					if (!lwRead< uint16_t >(stream, m_textureCoordinateSystem))
						return false;
				}
				//else
				//	System.err.println("Unknown TMAP chunk (" + sub.toString() + ")");
					
				stream->seek(IStream::SeekSet, tsub.end);
			}
		}
		else if (sub.tag == LwTags::PROJ)
			;
		else if (sub.tag == LwTags::IMAG)
		{
			if (!lwReadIndex(stream, m_textureImage))
				return false;
		}
		else if (sub.tag == LwTags::WRAP)
		{
			if (!lwRead< uint16_t >(stream, m_textureWidthWrap))
				return false;
			if (!lwRead< uint16_t >(stream, m_textureHeightWrap))
				return false;
		}
		else if (sub.tag == LwTags::WRPW)
			;
		else if (sub.tag == LwTags::WRPH)
			;
		else if (sub.tag == LwTags::VMAP)
		{
			if (!lwRead< std::wstring >(stream, m_textureVertexMap))
				return false;
		}
		else if (sub.tag == LwTags::AAST)
			;
		else if (sub.tag == LwTags::PIXB)
			;
		else if (sub.tag == LwTags::STCK)
			;
		else if (sub.tag == LwTags::TAMP)
			;
		//else
		//	System.err.println("Unknown BLOK chunk (" + sub.toString() + ")");
			
		stream->seek(IStream::SeekSet, sub.end);
	}

	return true;
}

	}
}
