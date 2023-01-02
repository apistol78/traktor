/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Test/Case.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.test.Case", Case, Object)

bool Case::execute(const IReport& infoReport, const IReport& errorReport)
{
	m_infoReport = &infoReport;
	m_errorReport = &errorReport;
	m_allocdelta = 0;

#if 0
	int64_t allocPre = (int64_t)Alloc::allocated();
#endif
    
	run();

#if 0
    int64_t allocPost = (int64_t)Alloc::allocated();
    int64_t allocBalance = (allocPost - allocPre) - m_allocdelta;
	if (allocBalance != 0)
	{
		StringOutputStream ss;
		ss << T_FILE_LINE_W << L" \"" << type_name(this) << L"\" failed; " << allocBalance << L" memory allocation leak."; \
		failed(ss.str());
	}
#endif

	m_infoReport = nullptr;
	m_errorReport = nullptr;
	return !m_failed;
}

void Case::succeeded(const std::wstring& message)
{
	int64_t allocPre = (int64_t)Alloc::allocated();

	if (m_infoReport)
		m_infoReport->report(message);
	//else
	//	log::info << message << Endl;

	int64_t allocPost = (int64_t)Alloc::allocated();
	m_allocdelta += (allocPost - allocPre);
}

void Case::failed(const std::wstring& message)
{
	int64_t allocPre = (int64_t)Alloc::allocated();

	if (m_errorReport)
		m_errorReport->report(message);
	//else
	//	log::error << message << Endl;

	int64_t allocPost = (int64_t)Alloc::allocated();
	m_allocdelta += (allocPost - allocPre);

	m_failed = true;
}

}
