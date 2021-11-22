#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IPipelineBuilder;
class IPipelineCommon;
class IPipelineDepends;
class IPipelineSettings;

	}

	namespace scene
	{

class SceneAsset;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS IScenePipelineOperator : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	virtual void destroy() = 0;

	virtual TypeInfoSet getOperatorTypes() const = 0;

	/*! Add dependencies which might get used by operator. */
	virtual void addDependencies(editor::IPipelineDepends* pipelineDepends) const = 0;

	/*! Transform scene asset. */
	virtual bool transform(
		editor::IPipelineCommon* pipelineCommon,
		const ISerializable* operatorData,
		SceneAsset* inoutSceneAsset
	) const = 0;

	/*! */
	virtual bool build(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* operatorData,
		const db::Instance* sourceInstance,
		SceneAsset* inoutSceneAsset,
		bool rebuild
	) const = 0;
};

	}
}
