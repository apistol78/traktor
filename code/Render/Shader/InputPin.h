/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_InputPin_H
#define traktor_render_InputPin_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Node;

/*! \brief Shader graph node input pin.
 * \ingroup Render
 */
class T_DLLCLASS InputPin
{
public:
	InputPin(Node* node, const std::wstring& name, bool optional);
	
	Node* getNode() const;
	
	const std::wstring& getName() const;

	bool isOptional() const;

private:
	Node* m_node;
	std::wstring m_name;
	bool m_optional;
};

	}
}

#endif	// traktor_render_InputPin_H
