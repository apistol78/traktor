/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

class Model;

/*!
 * \ingroup Model
 */
class T_DLLCLASS IModelOperation : public Object
{
	T_RTTI_CLASS;

protected:
	friend class Model;

	virtual bool required(const IModelOperation* lastOperation) const { return true; }

	virtual bool apply(Model& model) const = 0;
};

}
