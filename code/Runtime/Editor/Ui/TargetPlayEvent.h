/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Event.h"

namespace traktor::runtime
{

class TargetInstance;

/*!
 * \ingroup Runtime
 */
class TargetPlayEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	explicit TargetPlayEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

}
