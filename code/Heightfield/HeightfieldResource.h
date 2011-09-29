#ifndef traktor_hf_HeightfieldResource_H
#define traktor_hf_HeightfieldResource_H

#include <string>
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS HeightfieldResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldResource();

	virtual bool serialize(ISerializer& s);

	uint32_t getSize() const { return m_size; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	uint32_t getPatchDim() const { return m_patchDim; }

	uint32_t getDetailSkip() const { return m_detailSkip; }

private:
	friend class HeightfieldPipeline;

	uint32_t m_size;
	Vector4 m_worldExtent;
	uint32_t m_patchDim;
	uint32_t m_detailSkip;
};

	}
}

#endif	// traktor_hf_HeightfieldResource_H
