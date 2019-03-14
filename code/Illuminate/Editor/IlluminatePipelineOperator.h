#pragma once

#include "Scene/Editor/IScenePipelineOperator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminatePipelineOperator : public scene::IScenePipelineOperator
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getOperatorTypes() const override final;

	virtual bool build(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* operatorData, scene::SceneAsset* inoutSceneAsset) const override final;

private:
	std::wstring m_assetPath;
};

	}
}