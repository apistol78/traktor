#pragma once

#include "World/Editor/ILayerAttribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS ProbeLayerAttribute : public world::ILayerAttribute
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;
};

	}
}
