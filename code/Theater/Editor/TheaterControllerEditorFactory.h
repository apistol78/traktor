#ifndef traktor_theater_TheaterControllerEditorFactory_H
#define traktor_theater_TheaterControllerEditorFactory_H

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
	virtual const TypeInfoSet getControllerDataTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< scene::ISceneControllerEditor > createControllerEditor(const TypeInfo& controllerDataType) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_theater_TheaterControllerEditorFactory_H
