/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEditorFactory.h"

#include "Theater/TheaterEntityComponentData.h"
#include "Theater/TheaterWorldComponentData.h"
#include "Theater/Editor/TheaterEditor.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterEditorFactory", TheaterEditorFactory, scene::IComponentPanelEditorFactory)

const TypeInfoSet TheaterEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< TheaterWorldComponentData, TheaterEntityComponentData >();
}

Ref< scene::IComponentPanelEditor > TheaterEditorFactory::createComponentPanelEditor(const TypeInfo& componentDataType) const
{
	return new TheaterEditor();
}

}
