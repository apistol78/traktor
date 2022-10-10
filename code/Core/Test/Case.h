#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"
#include "Core/Misc/TString.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::test
{

/*! Unit test case.
 * \ingroup Core
 */
class T_DLLCLASS Case : public Object
{
	T_RTTI_CLASS;

public:
	struct IReport
	{
		virtual void report(const std::wstring& message) const = 0;
	};

	bool execute(const IReport& infoReport, const IReport& errorReport);

protected:
	virtual void run() = 0;

	void succeeded(const std::wstring& message);

	void failed(const std::wstring& message);

private:
	const IReport* m_infoReport = nullptr;
	const IReport* m_errorReport = nullptr;
	bool m_failed = false;
	int64_t m_allocdelta = 0;
};

#define CASE_ASSERT(condition) \
	{ \
		StringOutputStream ss; \
		bool __case__result__ = false; { __case__result__ = (bool)(condition); } \
		if (__case__result__) \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#condition) << L"\" succeeded."; \
			succeeded(ss.str()); \
		} \
		else \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#condition) << L"\" failed."; \
			failed(ss.str()); \
		} \
	}

#define CASE_ASSERT_EQUAL(expr1, expr2) \
	{ \
		StringOutputStream ss; \
		auto r1 = (expr1); \
		auto r2 = (expr2); \
		if (r1 != r2) \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" failed, " << r1 << L" should be equal to " << r2 << L"."; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" succeeded, result was as expected " << r1 << L"."; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_NOT_EQUAL(expr1, expr2) \
	{ \
		StringOutputStream ss; \
		if ((expr1) == (expr2)) \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" failed, not equal to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_COMPARE(expr1, expr2, compare) \
	{ \
		StringOutputStream ss; \
		auto r1 = (expr1); \
		auto r2 = (expr2); \
		if (!compare(r1, r2)) \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" failed, not equal to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_COMPARE_NOT(expr1, expr2, compare) \
	{ \
		StringOutputStream ss; \
		if (compare((expr1), (expr2))) \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" failed, (not) compared to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << T_FILE_LINE_W << L" \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

}
