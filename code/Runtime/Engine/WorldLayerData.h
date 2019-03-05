#pragma once

#include <map>
#include "Runtime/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class Scene;

	}

	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS WorldLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< scene::Scene > m_scene;
};

	}
}
