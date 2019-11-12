#include "Theater/Editor/TheaterControllerEditorFactory.h"
#include "Theater/Editor/TheaterControllerEditor.h"
#include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterControllerEditorFactory", TheaterControllerEditorFactory, scene::ISceneControllerEditorFactory)

const TypeInfoSet TheaterControllerEditorFactory::getControllerDataTypes() const
{
	return makeTypeInfoSet< TheaterControllerData >();
}

Ref< scene::ISceneControllerEditor > TheaterControllerEditorFactory::createControllerEditor(const TypeInfo& controllerDataType) const
{
	return new TheaterControllerEditor();
}

	}
}
