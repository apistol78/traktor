#ifndef traktor_render_Emitter_H
#define traktor_render_Emitter_H

#include <map>

namespace traktor
{

class Type;

	namespace render
	{
		namespace
		{

struct EmitterImpl;

		}

class Node;
class EmitterContext;

/*!
 * \ingroup SW
 */
class Emitter
{
public:
	Emitter();

	virtual ~Emitter();

	void emit(EmitterContext& c, Node* node);

private:
	std::map< const Type*, EmitterImpl* > m_emitters;
};

	}
}

#endif	// traktor_render_Emitter_H
