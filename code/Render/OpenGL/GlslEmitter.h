#ifndef traktor_render_GlslEmitter_H
#define traktor_render_GlslEmitter_H

#include <map>

namespace traktor
{

class Type;

	namespace render
	{

class Node;
class GlslContext;

struct Emitter;

/*!
 * \ingroup OGL
 */
class GlslEmitter
{
public:
	GlslEmitter();

	virtual ~GlslEmitter();

	void emit(GlslContext& c, Node* node);

private:
	std::map< const Type*, Emitter* > m_emitters;
};

	}
}

#endif	// traktor_render_GlslEmitter_H
