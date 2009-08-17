#include "Theater/Editor/TheaterControllerEditorFactory.h"
#include "Theater/Editor/TheaterControllerEditor.h"
#include "Theater/TheaterControllerData.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterControllerEditorFactory", TheaterControllerEditorFactory, scene::ISceneControllerEditorFactory)

const TypeSet TheaterControllerEditorFactory::getControllerDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< TheaterControllerData >());
	return typeSet;
}

scene::ISceneControllerEditor* TheaterControllerEditorFactory::createControllerEditor(const Type& controllerDataType) const
{
	return gc_new< TheaterControllerEditor >();
}

	}
}
