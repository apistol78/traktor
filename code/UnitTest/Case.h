/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Case_H
#define traktor_Case_H

#include <string>
#include <sstream>
#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

class Case
{
public:
	struct Context
	{
		uint32_t succeeded;
		uint32_t failed;

		Context()
		:	succeeded(0)
		,	failed(0)
		{
		}
	};

	virtual ~Case() {}

	void execute(Context& context);

protected:
	virtual void run() = 0;

	void succeeded(const std::wstring& message);

	void failed(const std::wstring& message);

private:
	Context* m_context;
};

#define CASE_ASSERT(condition) \
	{ \
		StringOutputStream ss; \
		if (!(condition)) \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#condition) << L"\" failed"; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#condition) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_EQUAL(expr1, expr2) \
	{ \
		StringOutputStream ss; \
		if ((expr1) != (expr2)) \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" failed, not equal to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_NOT_EQUAL(expr1, expr2) \
	{ \
		StringOutputStream ss; \
		if ((expr1) == (expr2)) \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" failed, not equal to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_COMPARE(expr1, expr2, compare) \
	{ \
		StringOutputStream ss; \
		if (!compare((expr1), (expr2))) \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" failed, compared to " << mbstows(#expr2); \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << Path(mbstows(__FILE__)).getFileName() << L" (" << __LINE__ << L") \"" << mbstows(#expr1) << L"\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

}

#endif	// traktor_Case_H
