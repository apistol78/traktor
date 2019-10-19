#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class InputPin;
class OutputPin;

/*! Shader graph edge.
 * \ingroup Render
 */
class T_DLLCLASS Edge : public ISerializable
{
	T_RTTI_CLASS;

public:
	Edge();

	explicit Edge(const OutputPin* source, const InputPin* destination);

	void setSource(const OutputPin* source);

	const OutputPin* getSource() const { return m_source; }

	void setDestination(const InputPin* destination);

	const InputPin* getDestination() const { return m_destination; }

	virtual void serialize(ISerializer& s) override final;

private:
	const OutputPin* m_source;
	const InputPin* m_destination;
};

	}
}

