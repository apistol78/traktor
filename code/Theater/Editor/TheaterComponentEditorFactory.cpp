/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterComponentEditor.h"
#include "Theater/Editor/TheaterComponentEditorFactory.h"
#include "Theater/TheaterComponentData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterComponentEditorFactory", TheaterComponentEditorFactory, scene::IWorldComponentEditorFactory)

const TypeInfoSet TheaterComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< TheaterComponentData >();
}

Ref< scene::IWorldComponentEditor > TheaterComponentEditorFactory::createComponentEditor(const TypeInfo& componentDataType) const
{
	return new TheaterComponentEditor();
}

}
