/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	return makeTypeInfoSet< TheaterControllerData >();
}

Ref< scene::ISceneControllerEditor > TheaterControllerEditorFactory::createControllerEditor(const TypeInfo& controllerDataType) const
{
	return new TheaterControllerEditor();
}

	}
}
