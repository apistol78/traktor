/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Constraints/Constraint.h>

#include "Core/Ref.h"
#include "Physics/Jolt/BodyJolt.h"
#include "Physics/Jolt/Types.h"

namespace traktor::physics
{

/*!
 * \ingroup Jolt
 */
template < typename Outer, typename Constraint >
class JointJolt : public Outer
{
public:
	explicit JointJolt(IWorldCallback* callback, Constraint* constraint, BodyJolt* body1, BodyJolt* body2)
	:	m_callback(callback)
	,	m_constraint(constraint)
	,	m_body1(body1)
	,	m_body2(body2)
	{
		m_constraint->AddRef();
		if (m_body1)
			m_body1->addConstraint(m_constraint);
		if (m_body2)
			m_body2->addConstraint(m_constraint);
	}

	virtual ~JointJolt()
	{
		destroy();
	}

	virtual void destroy() override
	{
		if (!m_constraint)
			return;

		if (m_body1)
			m_body1->removeConstraint(m_constraint);
		if (m_body2)
			m_body2->removeConstraint(m_constraint);

		IWorldCallback* callback = m_callback;
		Constraint* constraint = m_constraint;
		m_callback = nullptr;
		m_constraint = nullptr;
		m_body1 = nullptr;
		m_body2 = nullptr;

		if (callback)
			callback->destroyConstraint(this, constraint);

		constraint->Release();
	}

	virtual Body* getBody1() override final
	{
		return m_body1;
	}

	virtual Body* getBody2() override final
	{
		return m_body2;
	}

	virtual void* getInternal() override final
	{
		return m_constraint;
	}

protected:
	IWorldCallback* m_callback;
	Constraint* m_constraint;
	Ref< BodyJolt > m_body1;
	Ref< BodyJolt > m_body2;
};

}
