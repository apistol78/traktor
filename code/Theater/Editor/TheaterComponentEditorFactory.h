/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IWorldComponentEditorFactory.h"

namespace traktor::theater
{

/*!
 */
class TheaterComponentEditorFactory : public scene::IWorldComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const override final;

	virtual Ref< scene::IWorldComponentEditor > createComponentEditor(const TypeInfo& componentDataType) const override final;
};

}
