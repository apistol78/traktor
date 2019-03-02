#pragma once

#include "Render/IVolumeTexture.h"
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

class T_DLLCLASS VolumeTexturePs3 : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	VolumeTexturePs3(int32_t& counter);

	virtual ~VolumeTexturePs3();

	bool create(MemoryHeap* memoryHeap, const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const;

	virtual int32_t getHeight() const;

	virtual int32_t getDepth() const;

	void bind(StateCachePs3& stateCache, int stage, const SamplerStateGCM& samplerState);

private:
	MemoryHeapObject* m_data;
	CellGcmTexture m_texture;
	int32_t& m_counter;
};

	}
}
