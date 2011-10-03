#ifndef traktor_hf_HeightfieldCompositor_H
#define traktor_hf_HeightfieldCompositor_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Heightfield/HeightfieldTypes.h"
#include "Heightfield/Editor/Region.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace hf
	{

class HeightfieldLayer;
class IBrush;
class Region;

class T_DLLCLASS HeightfieldCompositor : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< HeightfieldCompositor > createFromInstance(const db::Instance* assetInstance, const std::wstring& assetPath);

	bool saveInstanceLayers(db::Instance* assetInstance) const;

	float getNearestHeight(float x, float z) const;

	float getBilinearHeight(float x, float z) const;

	bool queryRay(const Vector4& localRayOrigin, const Vector4& localRayDirection, Scalar& outDistance, Vector4* outPosition = 0) const;

	void begin();

	void strokeBrush(const Vector4& fromPosition, const Vector4& toPosition, const IBrush* brush, Region* outDirtyRegion = 0);

	void end(Region* outDirtyRegion = 0);

	void copyHeights(height_t* h) const;

	float worldToGridX(float x) const;

	float worldToGridZ(float z) const;

	float gridToWorldX(float x) const;

	float gridToWorldZ(float z) const;

	uint32_t getSize() const { return m_size; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	const HeightfieldLayer* getBaseLayer() const { return m_baseLayer; }

	const HeightfieldLayer* getOffsetLayer() const { return m_offsetLayer; }

	HeightfieldLayer* getOffsetLayer() { return m_offsetLayer; }

	HeightfieldLayer* getAccumLayer() { return m_accumLayer; }

	const HeightfieldLayer* getMergedLayer() const { return m_mergedLayer; }

private:
	uint32_t m_size;
	Vector4 m_worldExtent;
	Ref< HeightfieldLayer > m_baseLayer;
	Ref< HeightfieldLayer > m_offsetLayer;
	Ref< HeightfieldLayer > m_accumLayer;
	Ref< HeightfieldLayer > m_mergedLayer;
	Region m_dirtyDraw;

	void updateMergedLayer(const Region& r);
};

	}
}

#endif	// traktor_hf_HeightfieldCompositor_H
