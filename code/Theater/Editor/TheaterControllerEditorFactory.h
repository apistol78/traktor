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
	T_RTTI_CLASS(TheaterControllerEditorFactory)

public:
	virtual const TypeSet getControllerDataTypes() const;

	virtual Ref< scene::ISceneControllerEditor > createControllerEditor(const Type& controllerDataType) const;
};

	}
}

#endif	// traktor_theater_TheaterControllerEditorFactory_H
