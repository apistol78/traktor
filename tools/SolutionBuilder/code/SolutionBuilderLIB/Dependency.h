#ifndef Dependency_H
#define Dependency_H

#include <Core/Serialization/ISerializable.h>

class SolutionLoader;

class Dependency : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	enum Link
	{
		LnkNo,		//<! Do not link; only ensure dependency is built up-to-date.
		LnkYes,		//<! Link with product.
		LnkForce	//<! Link with product, MUST keep all symbols.
	};

	Dependency();

	void setInheritIncludePaths(bool inheritIncludePaths);

	bool getInheritIncludePaths() const;

	void setLink(Link link);

	Link getLink() const;

	virtual std::wstring getName() const = 0;

	virtual std::wstring getLocation() const = 0;

	virtual bool resolve(SolutionLoader* solutionLoader) = 0;

	virtual void serialize(traktor::ISerializer& s);

private:
	bool m_inheritIncludePaths;
	Link m_link;
};

#endif	// Dependency_H
