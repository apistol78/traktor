#include "Core/Io/IStream.h"
#include "Model/Formats/LwBlock.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwRead.h"
#include "Model/Formats/LwSurface.h"
#include "Model/Formats/LwTags.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwSurface", LwSurface, Object)

bool LwSurface::readLWO2(const LwChunk& chunk, IStream* stream)
{
	if (!lwRead< std::wstring >(stream, m_name))
		return false;

	if (!lwRead< std::wstring >(stream, m_source))
		return false;
		
	while (stream->tell() < chunk.end)
	{
		LwChunk sub;
		if (!sub.readSub(stream))
			return false;

		switch (sub.tag)
		{
		case LwTags::COLR:
			{
				float e[3];
				if (!lwRead< float >(stream, e, sizeof_array(e)))
					return false;

				m_color.val = Color4f(e[0], e[1], e[2], 1.0f);
			}
			break;

		case LwTags::LUMI:
			break;
		case LwTags::DIFF:
			break;
		case LwTags::SPEC:
			break;
		case LwTags::GLOS:
			break;
		case LwTags::REFL:
			break;
		case LwTags::RFOP:
			break;
		case LwTags::RIMG:
			break;
		case LwTags::RSAN:
			break;
		case LwTags::TRAN:
			break;
		case LwTags::TROP:
			break;
		case LwTags::TIMG:
			break;
		case LwTags::RIND:
			break;
		case LwTags::TRNL:
			break;
		case LwTags::BUMP:
			break;
		case LwTags::SMAN:
			break;
		case LwTags::SIDE:
			break;
		case LwTags::CLRH:
			break;
		case LwTags::CLRF:
			break;
		case LwTags::ADTR:
			break;
		case LwTags::SHRP:
			break;
		case LwTags::GVAL:
			break;
		case LwTags::LINE:
			break;
		case LwTags::ALPH:
			break;
		case LwTags::AVAL:
			break;
		case LwTags::CMNT:
			break;

		case LwTags::BLOK:
			{
				Ref< LwBlock > block = new LwBlock();
				if (!block->read(sub, stream))
					return false;

				//m_blocks.push_back(block);
			}
			break;
		}
		
		stream->seek(IStream::SeekSet, sub.end);
	}

	return true;
}

bool LwSurface::readLWOB(const LwChunk& chunk, IStream* stream)
{
	if (!lwRead< std::wstring >(stream, m_name))
		return false;

	while (stream->tell() < chunk.end)
	{
		LwChunk sub;
		if (!sub.readSub(stream))
			return false;
			
		if (sub.tag == LwTags::COLR)
		{
			float e[3];
			if (!lwRead< float >(stream, e, sizeof_array(e)))
				return false;

			m_color.val = Color4f(e[0], e[1], e[2], 1.0f);
		}
		else if (sub.tag == LwTags::BLOK)
		{
			Ref< LwBlock > block = new LwBlock();
			if (!block->read(sub, stream))
				return false;

			//m_blocks.push_back(block);
		}
		
		stream->seek(IStream::SeekSet, sub.end);
	}

	return true;
}

	}
}
