#ifndef Transformer_H
#define Transformer_H

#include <Core/Object.h>
#include <Core/Ref.h>

namespace traktor
{
	namespace script
	{

class IScriptContext;
class IScriptManager;

	}

	namespace xml
	{

class Document;
class Element;

	}
}

class Transformer : public traktor::Object
{
	T_RTTI_CLASS;

public:
	bool create(const std::wstring& script);

	void destroy();

	int32_t transform(traktor::xml::Document* document);

private:
	traktor::Ref< traktor::script::IScriptManager > m_scriptManager;
	std::wstring m_script;

	int32_t transform(traktor::script::IScriptContext* context, traktor::xml::Element* element);
};

#endif	// Transformer_H
