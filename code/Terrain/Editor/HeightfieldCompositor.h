#ifndef traktor_terrain_HeightfieldCompositor_H
#define traktor_terrain_HeightfieldCompositor_H

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace terrain
	{

class HeightfieldLayer;

class HeightfieldCompositor : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< HeightfieldCompositor > createFromInstance(const db::Instance* assetInstance, const std::wstring& assetPath);

	HeightfieldLayer* getBaseLayer();

	HeightfieldLayer* getOffsetLayer();

	Ref< HeightfieldLayer > mergeLayers() const;

	bool saveInstanceLayers(db::Instance* assetInstance) const;

private:
	Ref< HeightfieldLayer > m_baseLayer;
	Ref< HeightfieldLayer > m_offsetLayer;
};

	}
}

#endif	// traktor_terrain_HeightfieldCompositor_H
