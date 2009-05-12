#ifndef CompilerFormats_H
#define CompilerFormats_H

/*! \brief Compiler tool formats.
 *
 * \example
 * "/I{0}"
 * "-I {0}"
 */
class CompilerFormats : public traktor::Serializable
{
	T_RTTI_CLASS(CompilerFormats)

public:
	std::wstring formatIncludePath(const std::wstring& includePath) const;

	std::wstring formatDefine(const std::wstring& define) const;

	std::wstring formatDeleteFile(const std::wstring& fileName) const;

	std::wstring formatArchive()

	std::wstring getObjectSuffix() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_includePathFormat;
	std::wstring m_defineFormat;
	std::wstring m_deleteFileFormat;
};

#endif	// CompilerFormats_H
