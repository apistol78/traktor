#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace sb
	{

class SolutionLoader;

class T_DLLCLASS Dependency : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum Link
	{
		LnkNo,		//<! Do not link; only ensure dependency is built up-to-date.
		LnkYes,		//<! Link with product.
		LnkForce	//<! Link with product, MUST keep all symbols.
	};

	void setInheritIncludePaths(bool inheritIncludePaths);

	bool getInheritIncludePaths() const;

	void setLink(Link link);

	Link getLink() const;

	virtual std::wstring getName() const = 0;

	virtual std::wstring getLocation() const = 0;

	virtual bool resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader) = 0;

	virtual void serialize(ISerializer& s) override;

private:
	bool m_inheritIncludePaths = true;
	Link m_link = LnkYes;
};

	}
}

