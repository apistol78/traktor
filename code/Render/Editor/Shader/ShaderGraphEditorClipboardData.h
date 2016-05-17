#ifndef traktor_render_ShaderGraphEditorClipboardData_H
#define traktor_render_ShaderGraphEditorClipboardData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Node;
class Edge;

class T_DLLCLASS ShaderGraphEditorClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addNode(Node* node);

	void addEdge(Edge* edge);

	void setBounds(const ui::Rect& bounds);

	const RefArray< Node >& getNodes() const;

	const RefArray< Edge >& getEdges() const;

	const ui::Rect& getBounds() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	ui::Rect m_bounds;
};

	}
}

#endif	// traktor_render_ShaderGraphEditorClipboardData_H
