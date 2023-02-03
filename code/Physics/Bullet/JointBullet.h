/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <btBulletDynamicsCommon.h>
#include "Core/Ref.h"
#include "Core/Misc/InvokeOnce.h"
#include "Physics/Bullet/BodyBullet.h"
#include "Physics/Bullet/Types.h"

namespace traktor::physics
{

/*!
 * \ingroup Bullet
 */
struct JointSolver
{
	virtual ~JointSolver() {}

	virtual void prepare() {}

	virtual void update(float deltaTime) {}
};

/*!
 * \ingroup Bullet
 */
template < typename Outer, typename Constraint >
class JointBullet
:	public Outer
,	public JointSolver
{
public:
	explicit JointBullet(IWorldCallback* callback, Constraint* constraint, BodyBullet* body1, BodyBullet* body2)
	:	m_callback(callback)
	,	m_constraint(constraint)
	,	m_body1(body1)
	,	m_body2(body2)
	{
		if (m_body1)
			m_body1->addConstraint(m_constraint);
		if (m_body2)
			m_body2->addConstraint(m_constraint);
	}

	virtual ~JointBullet()
	{
		destroy();
	}

	virtual void destroy() override
	{
		if (m_body1)
			m_body1->removeConstraint(m_constraint);
		if (m_body2)
			m_body2->removeConstraint(m_constraint);

		invokeOnce< IWorldCallback, Joint*, btTypedConstraint* >(m_callback, &IWorldCallback::destroyConstraint, this, m_constraint);

		m_constraint = nullptr;
		m_body1 = nullptr;
		m_body2 = nullptr;
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
	Ref< BodyBullet > m_body1;
	Ref< BodyBullet > m_body2;
};

}
