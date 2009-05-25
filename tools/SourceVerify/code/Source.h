#ifndef Source_H
#define Source_H

#include <vector>
#include <string>
#include <Core/Object.h>
#include <Core/Io/Stream.h>

class Source : public traktor::Object
{
	T_RTTI_CLASS(Source)

public:
	struct Line
	{
		uint32_t line;
		std::wstring text;
	};

	bool create(traktor::Stream* file);

	const std::vector< Line >& getOriginalLines() const { return m_originalLines; }

	const std::vector< Line >& getUncommentedLines() const { return m_uncommentedLines; }

private:
	std::vector< Line > m_originalLines;
	std::vector< Line > m_uncommentedLines;
};

#endif	// Source_H
