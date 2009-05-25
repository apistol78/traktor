#ifndef traktor_Case_H
#define traktor_Case_H

#include <string>
#include <sstream>

namespace traktor
{

class Case
{
public:
	virtual void run() = 0;

protected:
	void succeeded(const std::string& message);

	void failed(const std::string& message);
};

#define CASE_ASSERT(condition) \
	{ \
		std::stringstream ss; \
		if (!(condition)) \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #condition << "\" failed"; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #condition << "\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_EQUAL(expr1, expr2) \
	{ \
		std::stringstream ss; \
		if ((expr1) != (expr2)) \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" failed, not equal to " << #expr2; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_NOT_EQUAL(expr1, expr2) \
	{ \
		std::stringstream ss; \
		if ((expr1) == (expr2)) \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" failed, not equal to " << #expr2; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

#define CASE_ASSERT_COMPARE(expr1, expr2, compare) \
	{ \
		std::stringstream ss; \
		if (!compare((expr1), (expr2))) \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" failed, compared to " << #expr2; \
			failed(ss.str()); \
		} \
		else \
		{ \
			ss << __FILE__ " (" << __LINE__ << ") \"" << #expr1 << "\" succeeded"; \
			succeeded(ss.str()); \
		} \
	}

}

#endif	// traktor_Case_H
