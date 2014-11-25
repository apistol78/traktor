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

class MemoryHeap;
class MemoryHeapObject;
class StateCachePs3;
		
class T_DLLCLASS SimpleTexturePs3 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTexturePs3(int32_t& counter);

	virtual ~SimpleTexturePs3();
	
	bool create(MemoryHeap* memoryHeap, const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	void bind(StateCachePs3& stateCache, int stage, const SamplerStateGCM& samplerState);

private:
	MemoryHeapObject* m_data;
	CellGcmTexture m_texture;
	int32_t& m_counter;
};
		
	}
}

#endif	// traktor_render_SimpleTexturePs3_H
