/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/IAccelerationStructure.h"

namespace traktor::render
{

/*!
 * \ingroup Render
 */
class AccelerationStructureVrfy : public IAccelerationStructure
{
	T_RTTI_CLASS;

public:
	explicit AccelerationStructureVrfy(IAccelerationStructure* wrappedAccelerationStructure);

	/*! */
	virtual void destroy() override final;

	IAccelerationStructure* getWrappedAS() const { return m_wrappedAccelerationStructure; }

private:
	Ref< IAccelerationStructure > m_wrappedAccelerationStructure;
};

}
