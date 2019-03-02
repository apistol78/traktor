#pragma once

#include "Scene/Editor/ISceneControllerEditorFactory.h"

namespace traktor
{
	namespace theater
	{

/*! \brief
 */
class TheaterControllerEditorFactory : public scene::ISceneControllerEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getControllerDataTypes() const override final;

	virtual Ref< scene::ISceneControllerEditor > createControllerEditor(const TypeInfo& controllerDataType) const override final;
};

	}
}

