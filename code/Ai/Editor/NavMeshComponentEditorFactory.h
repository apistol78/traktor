/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IComponentPanelEditorFactory.h"

namespace traktor::ai
{

/*! Navigation mesh entity scene editor factory.
 * \ingroup AI
 */
class NavMeshComponentEditorFactory : public scene::IComponentPanelEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const override final;

	virtual Ref< scene::IComponentPanelEditor > createComponentPanelEditor(const TypeInfo& componentDataType) const override final;
};

}
