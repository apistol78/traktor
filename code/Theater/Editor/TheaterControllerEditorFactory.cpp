/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TheaterControllerData >());
	return typeSet;
}

Ref< scene::ISceneControllerEditor > TheaterControllerEditorFactory::createControllerEditor(const TypeInfo& controllerDataType) const
{
	return new TheaterControllerEditor();
}

	}
}
