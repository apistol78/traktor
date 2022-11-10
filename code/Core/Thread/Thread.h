/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Ref.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! OS thread class.
 * \ingroup Core
 */
class T_DLLCLASS Thread : public IWaitable
{
public:
	enum Priority
	{
		Lowest = -2,
		Below = -1,
		Normal = 0,
		Above = 1,
		Highest = 2
	};

	bool start(Priority priority = Normal);

	bool stop(int32_t timeout = -1);

	bool pause();

	bool resume();

	bool resume(Priority priority);

	void sleep(int32_t duration);

	void yield();

	bool current() const;

	bool stopped() const;

	bool finished() const;

	virtual bool wait(int32_t timeout = -1) override final;

	uint32_t id() const { return m_id; }

	const wchar_t* name() const { return m_name; }

	int32_t hardwareCore() const { return m_hardwareCore; }

private:
	friend class ThreadManager;

	void* m_handle = nullptr;
	uint32_t m_id = 0;
	bool m_stopped = false;
	std::function< void() > m_fn;
	const wchar_t* const m_name = nullptr;
	int32_t m_hardwareCore = 0;

	explicit Thread(const std::function< void() >& fn, const wchar_t* const name, int32_t hardwareCore);

	~Thread();
};

}
