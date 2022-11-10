/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/TracerModel.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerModel", TracerModel, Object)

TracerModel::TracerModel(const model::Model* model, const Transform& transform)
:   m_model(model)
,	m_transform(transform)
{
}

    }
}