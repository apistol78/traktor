
alias "cpp" int32
{
	"int32_t",
	"int32_t",
	"traktor::Member< int32_t >"
}

alias "cpp" int32[]
{
	"std::vector< int32_t >",
	"const std::vector< int32_t >&",
	"traktor::MemberStlVector< int32_t >"
}

alias "cpp" int32[ N ]
{
	"int32_t",
	"const std::vector< int32_t >&",
	"traktor::MemberStaticArray< int32_t, %N >(\"%0\", m_%0)"
}

namespace std
{

alias "cpp" string
{
	"std::string",
	"const std::string&",
	"traktor::Member< std::string >"
}

alias "cpp" wstring
{
	"std::wstring",
	"const std::wstring&",
	"traktor::Member< std::wstring >"
}

alias "cpp" vector< A >
{
	"std::vector< %A.type >",
	"const std::vector< %A.type >&",
	"traktor::MemberStlVector< %A.type, %A.member >"
}

alias "cpp" pair< A, B >
{
	"std::pair< %A.type, %B.type >",
	"const std::pair< %A.type, %B.type >&",
	"traktor::MemberStlPair< %A.type, %B.type, %A.member, %B.member >"
}

alias "cpp" map< A, B >
{
	"std::map< %A.type, %B.type >",
	"const std::map< %A.type, %B.type >&",
	"traktor::MemberStlMap< %A.type, %B.type, traktor::MemberStlPair< %A.type, %B.type, %A.member, %B.member > >"
}

}
