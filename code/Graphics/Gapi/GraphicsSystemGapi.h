#ifndef traktor_graphics_GraphicsSystemGapi_H
#define traktor_graphics_GraphicsSystemGapi_H

#include "Core/Heap/Ref.h"
#include "Graphics/GraphicsSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_GAPI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class SurfaceGapi;

class T_DLLCLASS GraphicsSystemGapi : public GraphicsSystem
{
	T_RTTI_CLASS(GraphicsSystemGapi)

public:
	virtual bool getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const;

	virtual bool create(const CreateDesc& createDesc);

	virtual void destroy();

	virtual bool resize(int width, int height);

	virtual Surface* getPrimarySurface();

	virtual Surface* getSecondarySurface();

	virtual Surface* createOffScreenSurface(const SurfaceDesc& surfaceDesc);

	virtual void flip(bool waitVBlank);

private:
	Ref< SurfaceGapi > m_secondarySurface;
};

	}
}

#endif	// traktor_graphics_GraphicsSystemGapi_H
