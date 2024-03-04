/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

class Model;

}

namespace traktor::hf
{

class T_DLLCLASS ConvertHeightfield : public Object
{
    T_RTTI_CLASS;

public:
    /*! Convert heightfield into model.
     *
     * \param heightfield Heightfield to convert.
     * \param step Step size, sample heightfield every N step.
     * \return Converted model.
     */
    Ref< model::Model > convert(const Heightfield* heightfield, int32_t step) const;
};

}
