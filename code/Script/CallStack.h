#ifndef traktor_script_CallStack_H
#define traktor_script_CallStack_H

#include <list>
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
class T_DLLCLASS CallStack : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Local
	{
		std::wstring name;
		std::wstring value;
	};

	struct Frame
	{
		std::wstring name;
		Guid scriptId;
		uint32_t lineNumber;
		std::list< Local > locals;
	};

	void pushFrame(const Frame& frame);

	bool popFrame();

	const Frame& getCurrentFrame() const;

	const std::list< Frame >& getFrames() const;

	virtual bool serialize(ISerializer& s);

private:
	std::list< Frame > m_frames;
};

	}
}

#endif	// traktor_script_CallStack_H
