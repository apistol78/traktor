#pragma once

#include "Render/ICubeTexture.h"
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
		
class T_DLLCLASS CubeTexturePs3 : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTexturePs3(int32_t& counter);

	virtual ~CubeTexturePs3();
	
	bool create(MemoryHeap* memoryHeap, const CubeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	void bind(StateCachePs3& stateCache, int stage, const SamplerStateGCM& samplerState);

private:
	MemoryHeapObject* m_data;
	CellGcmTexture m_texture;
	int32_t& m_counter;
};
		
	}
}
