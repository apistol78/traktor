/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

/*! Cut alpha tested surfaces to their diffuse coverage mask.
 * \ingroup Model
 *
 * Each alpha tested triangle is clipped and tessellated to the covered region of
 * its diffuse map. Non alpha tested surfaces pass through unchanged.
 */
class T_DLLCLASS CutAlpha : public IModelOperation
{
	T_RTTI_CLASS;

protected:
	virtual bool apply(Model& model) const override final;
};

}
