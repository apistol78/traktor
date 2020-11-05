#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Scene/Editor/IScenePipelineOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class IEntityReplicator;

	}

	namespace shape
	{

class TracerProcessor;

class T_DLLCLASS BakePipelineOperator : public scene::IScenePipelineOperator
{
	T_RTTI_CLASS;

public:
	BakePipelineOperator();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual bool addDependencies(editor::IPipelineDepends* pipelineDepends, const ISerializable* operatorData, const scene::SceneAsset* sceneAsset) const override final;

	virtual bool build(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* operatorData,
		const db::Instance* sourceInstance,
		scene::SceneAsset* inoutSceneAsset,
		bool rebuild
	) const override final;

	static void setTracerProcessor(TracerProcessor* tracerProcessor);

	static TracerProcessor* getTracerProcessor();

private:
	static Ref< TracerProcessor > ms_tracerProcessor;

	std::wstring m_assetPath;
	const TypeInfo* m_tracerType;
	std::wstring m_compressionMethod;
	bool m_editor;
	SmallMap< const TypeInfo*, Ref< const scene::IEntityReplicator > > m_entityReplicators;
};

	}
}