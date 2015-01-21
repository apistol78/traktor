#ifndef traktor_render_IntrProgram_H
#define traktor_render_IntrProgram_H

#include <map>
#include <vector>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"
#include "Render/Sw/Core/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

	namespace render
	{

struct EmitterVariable;

/*! \brief Intermediate program representation.
 * \ingroup SW
 */
class T_DLLCLASS IntrProgram : public ISerializable
{
	T_RTTI_CLASS;

public:
	uint32_t addInstruction(const Instruction& instruction);

	uint32_t addConstant(const Vector4& value);

	void setInstruction(uint32_t offset, const Instruction& instruction);

	void setConstant(uint32_t index, const Vector4& value);

	inline const std::vector< Instruction >& getInstructions() const { return m_instructions; }

	inline const AlignedVector< Vector4 >& getConstants() const { return m_constants; }

	inline const Vector4& getConstant(uint32_t index) const { T_ASSERT (index < uint32_t(m_constants.size())); return m_constants[index]; }

	void dump(OutputStream& os, const std::map< std::wstring, EmitterVariable* >& uniforms) const;

	virtual void serialize(ISerializer& s);

private:
	std::vector< Instruction > m_instructions;
	AlignedVector< Vector4 > m_constants;
};

	}
}

#endif	// traktor_render_IntrProgram_H
