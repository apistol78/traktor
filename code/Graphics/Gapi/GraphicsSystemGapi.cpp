#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gx.h>
#include "Graphics/Gapi/GraphicsSystemGapi.h"
#include "Graphics/Gapi/SurfaceGapi.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace graphics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.graphics.GraphicsSystemGapi", GraphicsSystemGapi, GraphicsSystem)

bool GraphicsSystemGapi::getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const
{
	GXDisplayProperties displayProperties = GXGetDisplayProperties();

	DisplayMode dm =
	{
		displayProperties.cxWidth,
		displayProperties.cyHeight,
		0
	};

	if (displayProperties.ffFormat & kfDirect555 || displayProperties.ffFormat & kfDirect565)
		dm.bits = 16;
	else if (displayProperties.ffFormat & kfDirect888)
		dm.bits = 24;

	outDisplayModes.push_back(dm);
	return true;
}

bool GraphicsSystemGapi::create(const CreateDesc& createDesc)
{
	if (!GXOpenDisplay((HWND)createDesc.windowHandle, GX_FULLSCREEN))
	{
		log::error << L"Unable to open display, GXOpenDisplay failed" << Endl;
		return false;
	}

	GXDisplayProperties displayProperties = GXGetDisplayProperties();
	if (displayProperties.cxWidth != createDesc.displayMode.width || displayProperties.cyHeight != createDesc.displayMode.height)
	{
		log::error << L"Resolution must be " << uint32_t(displayProperties.cxWidth) << L"*" << uint32_t(displayProperties.cyHeight) << Endl;
		return false;
	}

	SurfaceDesc surfaceDesc;
	surfaceDesc.width = displayProperties.cxWidth;
	surfaceDesc.height = displayProperties.cyHeight;
	surfaceDesc.pixelFormat = createDesc.pixelFormat;

	m_secondarySurface = checked_type_cast< SurfaceGapi* >(createOffScreenSurface(surfaceDesc));
	if (!m_secondarySurface)
	{
		GXCloseDisplay();
		return false;
	}

	return true;
}

void GraphicsSystemGapi::destroy()
{
	GXCloseDisplay();
}

bool GraphicsSystemGapi::resize(int width, int height)
{
	return true;
}

Surface* GraphicsSystemGapi::getPrimarySurface()
{
	return 0;
}

Surface* GraphicsSystemGapi::getSecondarySurface()
{
	return m_secondarySurface;
}

Surface* GraphicsSystemGapi::createOffScreenSurface(const SurfaceDesc& surfaceDesc)
{
	return gc_new< SurfaceGapi >(surfaceDesc);
}

void GraphicsSystemGapi::flip(bool waitVBlank)
{
	GXDisplayProperties displayProperties = GXGetDisplayProperties();

	void* videoMemory = GXBeginDraw();

	SurfaceDesc surfaceDesc;
	void* secondaryMemory = m_secondarySurface->lock(surfaceDesc);

	T_ASSERT (surfaceDesc.width >= displayProperties.cxWidth);
	T_ASSERT (surfaceDesc.height >= displayProperties.cyHeight);

	int pitchX = getByteSize(surfaceDesc.pixelFormat);
	int pitchY = surfaceDesc.pitch;

	unsigned char* vmp = static_cast< unsigned char* >(videoMemory);
	unsigned char* smp = static_cast< unsigned char* >(secondaryMemory);

	for (DWORD y = 0; y < displayProperties.cyHeight; ++y)
	{
		for (DWORD x = 0; x < displayProperties.cxWidth; ++x)
		{
			unsigned char rgb[3];

			switch (surfaceDesc.pixelFormat)
			{
			case PfeR5G5B5:
				break;

			case PfeA1R5G5B5:
				break;

			case PfeR5G6B5:
				break;

			case PfeR8G8B8:
			case PfeA8R8G8B8:
				rgb[0] = smp[2];
				rgb[1] = smp[1];
				rgb[2] = smp[0];
				break;
			}

			if (displayProperties.ffFormat & kfDirect555)
			{
				*reinterpret_cast< unsigned short* >(vmp) = 
					((rgb[0] & 0xf8) << 7) |
					((rgb[1] & 0xf8) << 2) |
					 (rgb[2] >> 3);
			}
			else if (displayProperties.ffFormat & kfDirect565)
			{
				*reinterpret_cast< unsigned short* >(vmp) = 
					((rgb[0] & 0xf8) << 8) |
					((rgb[1] & 0xfc) << 3) |
					 (rgb[2] >> 3);
			}
			else if (displayProperties.ffFormat & kfDirect888)
			{
				vmp[0] = rgb[2];
				vmp[1] = rgb[1];
				vmp[2] = rgb[0];
			}

			vmp += displayProperties.cbxPitch;
			smp += pitchX;
		}

		vmp += displayProperties.cbyPitch - (displayProperties.cbxPitch * displayProperties.cxWidth);
		smp += pitchY - surfaceDesc.pitch;
	}

	m_secondarySurface->unlock();

	GXEndDraw();
}

	}
}
