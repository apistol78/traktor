#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IPipelineSettings;

	}

	namespace scene
	{

class SceneAsset;

/*! \brief
 * \ingroup Scene
 */
class T_DLLCLASS IScenePipelineOperator : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	virtual void destroy() = 0;

	virtual TypeInfoSet getOperatorTypes() const = 0;

	virtual bool build(SceneAsset* inoutSceneAsset) const = 0;
};

	}
}