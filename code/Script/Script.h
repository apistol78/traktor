#ifndef traktor_script_Script_H
#define traktor_script_Script_H

#include "Core/Serialization/Serializable.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS Script : public Serializable
{
	T_RTTI_CLASS(Script)

public:
	Script();

	Script(const std::wstring& text);

	void addDependency(const Guid& dependency);

	std::vector< Guid >& getDependencies();

	const std::vector< Guid >& getDependencies() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

private:
	std::vector< Guid > m_dependencies;
	std::wstring m_text;
};

	}
}

#endif	// traktor_script_Script_H
