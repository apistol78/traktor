#ifndef ActiveHtmlFilter_H
#define ActiveHtmlFilter_H

#include <Core/Object.h>
#include <Core/Ref.h>
#include <Script/IScriptManager.h>

class ActiveHtmlFilter : public traktor::Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	std::wstring generate(const std::wstring& source) const;

private:
	traktor::Ref< traktor::script::IScriptManager > m_scriptManager;
};

#endif	// ActiveHtmlFilter_H
