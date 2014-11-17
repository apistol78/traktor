#include <stack>
#include "Drawing/Formats/ImageFormatGif.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/Palette.h"
#include "Drawing/PixelFormat.h"
#include "Core/Math/Color4f.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace drawing
	{
	
namespace
{

#pragma pack(1)

struct LogicalScreenDesc
{
	uint16_t width;
	uint16_t height;
	uint8_t packedFields;
	uint8_t background;
	uint8_t pixelAspectRatio;
};

struct PaletteEntry
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
};

struct GraphicControlExt
{
	uint8_t blockSize;
	uint8_t packedFields;
	uint16_t delay;
	uint8_t transparent;
};

struct IdTag
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint8_t packedFields;
};

#pragma pack()

void decodeLzw(const std::vector< uint8_t >& in, int initialCodeSize, std::vector< uint8_t >& out)
{
	struct Dictionary
	{
		uint8_t color;
		uint16_t parent;
	};
	
	Dictionary dict[4096];
	
	int codeSize = initialCodeSize + 1;		// Current code size.
	
	unsigned clearCode = 1 << initialCodeSize;	// Clear code
	unsigned endCode = clearCode + 1;		// End code
	
	int slot = clearCode + 2;			// Last read dictionary slot.
	int first = slot;
	
	for (int i = 0; i < 4096; ++i)
	{
		dict[i].color = i;
		dict[i].parent = 0;
	}

	unsigned code, outcode, oldcode = 0;
	
	std::stack< uint16_t > stack;

	for (unsigned offset = 0; offset <= (in.size() << 3) - codeSize; )
	{
		code = *reinterpret_cast< const unsigned* >(&in[offset / 8]);
		code >>= offset & 7;
		code &= (1 << codeSize) - 1;
	
		offset += codeSize;
		
		if (code == endCode)
			break;
		
		if (code == clearCode)
		{
			codeSize = initialCodeSize + 1;
			slot = first;
			oldcode = code;
			continue;
		}
		
		if (int(code) < slot)
		{
			outcode = code;
		}
		else
		{
			stack.push(oldcode);
			outcode = oldcode;
		}
		
		//while (dict[outcode].parent)
		while (int(outcode) >= first)
		{
			stack.push(outcode);
			outcode = dict[outcode].parent;
		}
		
		stack.push(outcode);
		
		if (oldcode != clearCode)
		{
			dict[slot].color = outcode;
			dict[slot].parent = oldcode;
			if (++slot >= (1 << codeSize))
			{
				if (codeSize < 12)
					codeSize++;
			}
		}
				
		oldcode = code;
		
		while (!stack.empty())
		{
			unsigned code2 = stack.top();
			out.push_back(dict[code2].color);
			stack.pop();
		}
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatGif", ImageFormatGif, IImageFormat)

Ref< Image > ImageFormatGif::read(IStream* stream)
{
	Ref< Image > image;
	
	char signature[6];
	if (stream->read(signature, sizeof(signature)) != sizeof(signature))
		return 0;
	
	if (std::string(signature, &signature[6]) != "GIF87a" && std::string(signature, &signature[6]) != "GIF89a")
		return 0;
	
	LogicalScreenDesc lsd;
	if (stream->read(&lsd, sizeof(lsd)) != sizeof(lsd))
		return 0;
		
	int bpp = (lsd.packedFields & 0x07) + 1;
	
	Ref< Palette > globalPalette = new Palette(1 << bpp);
	if (lsd.packedFields & 0x80)
	{
		for (int i = 0; i < (1 << bpp); ++i)
		{
			PaletteEntry pe;
			if (stream->read(&pe, sizeof(PaletteEntry)) != sizeof(PaletteEntry))
				return 0;
			globalPalette->set(i,
				Color4f(
					pe.r / 255.0f,
					pe.g / 255.0f,
					pe.b / 255.0f
				)
			);
		}
	}
	else
	{
		for (int i = 0; i < (1 << bpp); ++i)
		{
			globalPalette->set(
				i,
				Color4f(
					i / float((1 << bpp) - 1),
					i / float((1 << bpp) - 1),
					i / float((1 << bpp) - 1)
				)
			);
		}
	}
	
	while (stream->available() > 0)
	{
		Ref< Palette > palette = globalPalette;
		
		unsigned char type;
		if (stream->read(&type, sizeof(type)) != sizeof(type))
			return 0;
			
		if (type == 0x21)	// Extension block
		{
			if (stream->read(&type, sizeof(type)) != sizeof(type))
				return 0;
			switch (type)
			{
			case 0xf9:
				{
					GraphicControlExt gce;
					if (stream->read(&gce, sizeof(gce)) != sizeof(gce))
						return 0;
					
					char dummy;
					stream->read(&dummy, sizeof(dummy));
				}
				break;

			default:
				{
					unsigned char blockSize;
					if (stream->read(&blockSize, sizeof(blockSize)) != sizeof(blockSize))
						return 0;
						
					for (int i = 0; i < blockSize; ++i)
					{
						char dummy;
						stream->read(&dummy, sizeof(dummy));
					}
				}
				break;
			}
		}
		else if (type == 0x2c)	// Image separator
		{
			IdTag id;
			if (stream->read(&id, sizeof(id)) != sizeof(id))
				return 0;
			
			if (id.packedFields & 0x80)	// Local palette
			{
				palette = new Palette(1 << bpp);
				for (int i = 0; i < (1 << bpp); ++i)
				{
					PaletteEntry pe;
					if (stream->read(&pe, sizeof(PaletteEntry)) != sizeof(PaletteEntry))
						return 0;
					palette->set(i,
						Color4f(
							pe.r / 255.0f,
							pe.g / 255.0f,
							pe.b / 255.0f
						)
					);
				}
			}
			
			uint8_t initialCodeSize;
			if (stream->read(&initialCodeSize, sizeof(initialCodeSize)) != sizeof(initialCodeSize))
				return 0;
			if (initialCodeSize < 2 || initialCodeSize > 9)
				return 0;

			std::vector< uint8_t > compressed;
			for (;;)
			{
				uint8_t blockSize;
				
				if (stream->read(&blockSize, sizeof(blockSize)) != sizeof(blockSize))
					return 0;
				
				if (!blockSize)
					break;
				
				size_t end = compressed.size();
				compressed.resize(end + blockSize);
				
				if (stream->read(&compressed[end], blockSize) != blockSize)
					return 0;
			}
			
			std::vector< uint8_t > decompressed;
			decodeLzw(compressed, initialCodeSize, decompressed);
			decompressed.resize(id.width * id.height);

			bool interleaved = bool((id.packedFields & 0x40) != 0);
			int il[] = { 0, 8, 4, 8, 2, 4, 1, 2 };
			int ip = 0;

			image = new Image(PixelFormat::getP8(), id.width, id.height, palette);
			uint8_t* dst = static_cast< uint8_t* >(image->getData());
			
			for (int y = 0, yy = il[0]; y < id.height; ++y)
			{
				for (int x = 0; x < id.width; ++x)
				{
					dst[x + yy * id.width] = decompressed[x + y * id.width];
				}
				if (interleaved)
				{
					if ((yy += il[ip + 1]) >= id.height)
					{
						ip += 2;
						yy = il[ip];
					}
				}
				else
					++yy;
			}
			
			break;
		}
		else if (type == 0x3b)	// End of GIF
		{
			break;
		}
		else
		{
			log::warning << L"Unknown GIF block " << type << Endl;
		}
	}
	
	return image;
}

bool ImageFormatGif::write(IStream* stream, Image* image)
{
	return false;
}

	}
}
