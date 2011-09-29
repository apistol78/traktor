#ifndef traktor_hf_HeightfieldCompositor_H
#define traktor_hf_HeightfieldCompositor_H

#include "Core/Object.h"

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

class T_DLLCLASS HeightfieldCompositor : public Object
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

#endif	// traktor_hf_HeightfieldCompositor_H
