/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

/*!
 * \ingroup Model
 */
class T_DLLCLASS UnwrapUV : public IModelOperation
{
	T_RTTI_CLASS;

public:
	explicit UnwrapUV(int32_t channel, uint32_t textureSize);

protected:
	virtual bool apply(Model& model) const override final;

private:
	int32_t m_channel;
	uint32_t m_textureSize;
};

}
