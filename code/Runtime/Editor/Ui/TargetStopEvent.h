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

namespace traktor
{
	namespace runtime
	{

class TargetInstance;

/*! \brief
 * \ingroup Runtime
 */
class TargetStopEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetStopEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex);

	TargetInstance* getInstance() const;

	int32_t getConnectionIndex() const;

private:
	Ref< TargetInstance > m_instance;
	int32_t m_connectionIndex;
};

	}
}

