/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Test/Case.h"

namespace traktor::model
{

class ModelAdjacency;

}

namespace traktor::model::test
{

class CaseModelAdjacency : public traktor::test::Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;

private:
	void checkSharingEdges(const ModelAdjacency* ma);
};

}
