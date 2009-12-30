#ifndef traktor_render_SimpleTexturePs3_H
#define traktor_render_SimpleTexturePs3_H

#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class LocalMemoryObject;
		
class T_DLLCLASS SimpleTexturePs3 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTexturePs3();

	virtual ~SimpleTexturePs3();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	void bind(int stage, const SamplerState& samplerState);

private:
	LocalMemoryObject* m_data;
	CellGcmTexture m_texture;
};
		
	}
}

#endif	// traktor_render_SimpleTexturePs3_H
