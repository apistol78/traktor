/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace model
	{

/*!
 * \ingroup Model
 */
class T_DLLCLASS NormalizeTexCoords : public IModelOperation
{
	T_RTTI_CLASS;

public:
    explicit NormalizeTexCoords(
		uint32_t channel,
		float marginU,
		float marginV,
		float stepU,
		float stepV
	);

protected:
	virtual bool apply(Model& model) const override final;

private:
    uint32_t m_channel;
    float m_marginU;
    float m_marginV;
	float m_stepU;
	float m_stepV;
};

	}
}

