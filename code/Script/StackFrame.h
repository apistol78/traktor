#ifndef traktor_script_StackFrame_H
#define traktor_script_StackFrame_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Call stack debug information.
 * \ingroup Script
 */
class T_DLLCLASS StackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	StackFrame();

	void setScriptId(const Guid& scriptId);

	const Guid& getScriptId() const;

	void setFunctionName(const std::wstring& functionName);

	const std::wstring& getFunctionName() const;

	void setLine(uint32_t line);

	uint32_t getLine() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_scriptId;
	std::wstring m_functionName;
	uint32_t m_line;
};

	}
}

#endif	// traktor_script_StackFrame_H
