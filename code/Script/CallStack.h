#ifndef traktor_script_CallStack_H
#define traktor_script_CallStack_H

#include <list>
#include "Core/Guid.h"
#include "Core/RefArray.h"
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

class Local;

/*! \brief Call stack debug information.
 * \ingroup Script
 */
class T_DLLCLASS CallStack : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Frame
	{
		Guid scriptId;
		std::wstring scriptName;
		std::wstring functionName;
		uint32_t line;
		RefArray< Local > locals;

		Frame();
	};

	void pushFrame(const Frame& frame);

	bool popFrame();

	const Frame& getCurrentFrame() const;

	const std::list< Frame >& getFrames() const;

	virtual void serialize(ISerializer& s);

private:
	std::list< Frame > m_frames;
};

	}
}

#endif	// traktor_script_CallStack_H
