#ifndef traktor_terrain_HeightfieldResource_H
#define traktor_terrain_HeightfieldResource_H

#include <string>
#include "Core/Serialization/Serializable.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS HeightfieldResource : public Serializable
{
	T_RTTI_CLASS(HeightfieldResource)

public:
	HeightfieldResource();

	virtual bool serialize(Serializer& s);

	inline uint32_t getSize() const { return m_size; }

	inline const Vector4& getWorldExtent() const { return m_worldExtent; }

	inline uint32_t getPatchDim() const { return m_patchDim; }

	inline uint32_t getDetailSkip() const { return m_detailSkip; }

private:
	friend class HeightfieldPipeline;

	uint32_t m_size;
	Vector4 m_worldExtent;
	uint32_t m_patchDim;
	uint32_t m_detailSkip;
};

	}
}

#endif	// traktor_terrain_HeightfieldResource_H
