<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:wix="http://schemas.microsoft.com/wix/2006/wi"
    xmlns="http://schemas.microsoft.com/wix/2006/wi"

    version="1.0" 
    exclude-result-prefixes="xsl wix"
>

    <xsl:output method="xml" indent="yes" omit-xml-declaration="yes" />

    <xsl:strip-space elements="*" />

    <!--
    Find all <Component> elements with <File> elements with Source="" attributes ending in ".exe" and tag it with the "LibToRemove" key.

    <Component Id="cmpSYYKP6B1M7WSD5KLEQ7PZW4YLOPYG61L" Directory="INSTALLDIR" Guid="*">
        <File Id="filKUS7ZRMJ0AOKDU6ATYY6IRUSR2ECPDFO" KeyPath="yes" Source="!(wix.StagingAreaPath)\ProofOfPEqualsNP.exe" />
    </Component>

    Because WiX's Heat.exe only supports XSLT 1.0 and not XSLT 2.0 we cannot use `ends-with( haystack, needle )` (e.g. `ends-with( wix:File/@Source, '.exe' )`...
    ...but we can use this longer `substring` expression instead (see https://github.com/wixtoolset/issues/issues/5609 )
    -->
    <xsl:key
        name="LibToRemove"
        match="wix:Component[ substring( wix:File/@Source, string-length( wix:File/@Source ) - 3 ) = '.lib' ]"
        use="@Id"
    /> <!-- Get the last 4 characters of a string using `substring( s, len(s) - 3 )`, it uses -3 and not -4 because XSLT uses 1-based indexes, not 0-based indexes. -->

    <!-- We can also remove .pdb files too, for example: -->
    <xsl:key
        name="PdbToRemove"
        match="wix:Component[ substring( wix:File/@Source, string-length( wix:File/@Source ) - 3 ) = '.pdb' ]"
        use="@Id"
    />

    <!-- By default, copy all elements and nodes into the output... -->
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()" />
        </xsl:copy>
    </xsl:template>

    <!-- ...but if the element has the "LibToRemove" key then don't render anything (i.e. removing it from the output) -->
    <xsl:template match="*[ self::wix:Component or self::wix:ComponentRef ][ key( 'LibToRemove', @Id ) ]" />

    <xsl:template match="*[ self::wix:Component or self::wix:ComponentRef ][ key( 'PdbToRemove', @Id ) ]" />

    <xsl:template match="wix:File">
        <xsl:copy>
            <xsl:apply-templates select="@*" />
            <xsl:attribute name="ReadOnly">
                <xsl:text>yes</xsl:text>
            </xsl:attribute>
            <xsl:apply-templates select="*" />
        </xsl:copy>
    </xsl:template>

</xsl:stylesheet>