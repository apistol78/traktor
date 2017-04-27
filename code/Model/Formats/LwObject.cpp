/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <utility>
#include "Core/Io/IStream.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwClip.h"
#include "Model/Formats/LwLayer.h"
#include "Model/Formats/LwObject.h"
#include "Model/Formats/LwPolygon.h"
#include "Model/Formats/LwRead.h"
#include "Model/Formats/LwSurface.h"
#include "Model/Formats/LwTags.h"
#include "Model/Formats/LwVMad.h"
#include "Model/Formats/LwVMap.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwObject", LwObject, Object)

bool LwObject::read(IStream* stream)
{
	LwChunk form;

	if (!form.read(stream))
		return false;
	if (form.tag != LwTags::FORM)
		return false;

	if (!lwRead< int32_t >(stream, m_objectType))
		return false;
	if (m_objectType != LwTags::LWOB && m_objectType != LwTags::LWO2)
		return false;
		
	Ref< LwLayer > layer = new LwLayer();
	int32_t nlayers = 0;

	while (stream->tell() < form.end)
	{
		LwChunk chunk;

		if (!chunk.read(stream))
			return false;

		if (chunk.tag == LwTags::LAYR)
		{
			if (nlayers > 0)
			{
				m_layers.push_back(layer);
				layer = new LwLayer();
			}

			++nlayers;

			if (!layer->read(chunk, stream))
				return false;
		}
		else if (chunk.tag == LwTags::PNTS)
		{
			for (uint32_t i = 0; i < chunk.length / 12; ++i)
			{
				float e[3];
				if (!lwRead< float >(stream, e, sizeof_array(e)))
					return false;
				layer->m_points.push_back(Vector4(e[0], e[1], e[2], 1.0f));
			}
		}
		else if (chunk.tag == LwTags::VMAP)
		{
			Ref< LwVMap > vmap = new LwVMap();;
			if (!vmap->read(chunk, stream))
				return false;

			layer->m_vmaps.push_back(vmap);
		}
		else if (chunk.tag == LwTags::VMAD)
		{
			Ref< LwVMad > vmad = new LwVMad();
			if (!vmad->read(chunk, stream))
				return false;

			layer->m_vmads.push_back(vmad);
		}
		else if (chunk.tag == LwTags::POLS)
		{
			if (m_objectType == LwTags::LWO2)
			{
				int32_t type;
				if (!lwRead< int32_t >(stream, type))
					return false;

				if (type == LwTags::FACE || type == LwTags::PTCH)
				{
					while (stream->tell() < chunk.end)
					{
						LwPolygon polygon;

						polygon.type = type;

						if (!polygon.readLWO2(stream))
							return false;

						layer->m_polygons.push_back(polygon);
					}
				}
				//else
				//{
				//	System.err.println("Unsupported polygon type " + LwTags::toString(type));
				//}
			}
			else if (m_objectType == LwTags::LWOB)
			{
				while (stream->tell() < chunk.end)
				{
					LwPolygon polygon;
					if (!polygon.readLWOB(stream))
						return false;

					layer->m_polygons.push_back(polygon);
				}
			}
		}
		else if (chunk.tag == LwTags::TAGS || chunk.tag == LwTags::SRFS)
		{
			while (stream->tell() < chunk.end)
			{
				std::wstring tag;
				if (!lwRead< std::wstring >(stream, tag))
					return false;

				m_tags.push_back(tag);
			}
		}
		else if (chunk.tag == LwTags::PTAG)
		{
			uint32_t type;
			if (!lwRead< uint32_t >(stream, type))
				return false;

			while (stream->tell() < chunk.end)
			{
				uint32_t ptag;
				if (!lwReadIndex(stream, ptag))
					return false;

				uint16_t tags;
				if (!lwRead< uint16_t >(stream, tags))
					return false;

				if (type == LwTags::SURF)
				{
					if (ptag < layer->m_polygons.size())
						layer->m_polygons[ptag].surfaceTag = tags;
				}
				else if (type == LwTags::SMGP)
				{
					if (ptag < layer->m_polygons.size())
						layer->m_polygons[ptag].smoothGroup = tags;
				}
			}
		}
		else if (chunk.tag == LwTags::CLIP)
		{
			Ref< LwClip > clip = new LwClip();
			if (!clip->read(chunk, stream))
				return false;

			m_clips.push_back(clip);
		}
		else if (chunk.tag == LwTags::SURF)
		{
			Ref< LwSurface > surface = new LwSurface();
			if (m_objectType == LwTags::LWO2)
			{
				if (!surface->readLWO2(chunk, stream))
					return false;
			}
			else
			{
				if (!surface->readLWOB(chunk, stream))
					return false;
			}

			m_surfaces.push_back(surface);
		}
			
		stream->seek(IStream::SeekSet, chunk.end);
	}

	m_layers.push_back(layer);

	// Fixup polygon surfaces.
	for (RefArray< LwLayer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		LwLayer* layer = *i;
		for (AlignedVector< LwPolygon >::iterator j = layer->m_polygons.begin(); j != layer->m_polygons.end(); ++j)
		{
			const std::wstring& surfaceName = m_tags[j->surfaceTag];
			for (RefArray< LwSurface >::const_iterator k = m_surfaces.begin(); k != m_surfaces.end(); ++k)
			{
				if((*k)->getName() == surfaceName)
				{
					j->surface = *k;
					j->surfaceIndex = std::distance< RefArray< LwSurface >::const_iterator >(m_surfaces.begin(), k);
					break;
				}
			}
		}
	}

	return true;
}

	}
}
