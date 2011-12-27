#ifndef traktor_render_Constant_H
#define traktor_render_Constant_H

#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

class Constant
{
public:
	Constant();

	Constant(PinType type);

	explicit Constant(float scalar1);

	Constant cast(PinType type) const;

	PinType getType() const;

	int32_t getWidth() const;

	bool isZero() const;

	bool isOne() const;

	float& operator [] (int32_t index);

	float operator [] (int32_t index) const;

private:
	PinType m_type;
	float m_data[4];
};

	}
}

#endif	// traktor_render_Constant_H
