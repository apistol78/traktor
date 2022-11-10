/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace scene
	{

/*! \brief
 */
class DefaultEntityEditorFactory : public IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const override final;

	virtual Ref< IEntityEditor > createEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter) const override final;
};

	}
}

