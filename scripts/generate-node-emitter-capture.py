#!/usr/bin/env python3
"""
Generate code/MCP/Server/Editor/NodeEmitterCapture.cpp: source-derived shader
node metadata for the MCP server, so it can be exposed at runtime WITHOUT the
Traktor source tree being present.

Captures, per shader node type:
  - the literal GLSL emitter function body, sliced from
    code/Render/Editor/Glsl/GlslEmitter.cpp (nodeEmitterCapture);
  - a best-effort one-line GLSL expression (nodeEmitterSummary);
  - whether the node class is tagged T_DEPRECATED in its header
    (nodeDeprecatedTypes).

Run from the repository root:  python3 scripts/generate-node-emitter-capture.py
Re-run whenever GlslEmitter.cpp or the node headers change.
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "code/Render/Editor/Glsl/GlslEmitter.cpp")
OUT = os.path.join(ROOT, "code/MCP/Server/Editor/NodeEmitterCapture.cpp")
# Headers that declare shader graph node classes; scanned for T_DEPRECATED tags.
NODE_HEADER_GLOBS = [
    "code/Render/Editor/Shader/*.h",
    "code/Render/Editor/*.h",
]
DELIM = "NODESRC"


def extract_functions(lines):
    """Return { function_name: source_text } for each top-level emit function."""
    funcs = {}
    sig = re.compile(r"^bool\s+(emit\w+)\s*\(\s*GlslContext")
    i = 0
    n = len(lines)
    while i < n:
        m = sig.match(lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        start = i
        # Find the opening brace line (next line that is exactly '{').
        j = i + 1
        while j < n and lines[j].rstrip() != "{":
            # Bail if another signature appears before the body opens.
            if sig.match(lines[j]):
                break
            j += 1
        if j >= n or lines[j].rstrip() != "{":
            i += 1
            continue
        # Capture until the matching column-0 '}'. Inner braces are indented,
        # so the first line equal to '}' closes the function.
        k = j + 1
        while k < n and lines[k].rstrip() != "}":
            k += 1
        if k >= n:
            break
        body = "".join(lines[start:k + 1]).rstrip("\n")
        funcs[name] = body
        i = k + 1
    return funcs


def extract_registrations(text):
    """Return { node_type_short_name: function_name } from the emitter map."""
    reg = re.compile(
        r"m_emitters\[\s*&\s*type_of<\s*(\w+)\s*>\(\)\s*\]\s*=\s*new\s+EmitterCast<\s*\w+\s*>\(\s*(\w+)\s*\)"
    )
    mapping = {}
    for m in reg.finditer(text):
        mapping[m.group(1)] = m.group(2)
    return mapping


def summarize(body):
    """Best-effort one-line GLSL expression for a node, or None if not cleanly
    expressible. Reconstructs the single `assign(f, out) << ...;` statement,
    substituting emitInput variables with their pin names. Bails on anything
    that isn't a single straightforward assignment of literals + input pins."""
    vars = {}
    for m in re.finditer(r"(\w+)\s*=\s*cx\.emitInput\(\s*node\s*,\s*L\"([^\"]+)\"", body):
        vars[m.group(1)] = m.group(2)

    om = re.search(r"cx\.emitOutput\(\s*node\s*,\s*L\"([^\"]+)\"", body)
    out_name = om.group(1) if om else "Output"

    assign_lines = [ln.strip() for ln in body.splitlines() if re.search(r"\bassign\s*\(\s*f\s*,", ln)]
    if len(assign_lines) != 1:
        return None

    m = re.search(r"assign\s*\(\s*f\s*,\s*\w+\s*\)\s*<<(.*)$", assign_lines[0])
    if not m:
        return None

    chain = re.sub(r";\s*$", "", m.group(1))  # drop the C++ statement terminator
    rhs = []
    for raw in chain.split("<<"):
        tok = raw.strip()
        if not tok or tok == "Endl":
            continue
        lit = re.fullmatch(r'L"(.*)"', tok)
        if lit:
            rhs.append(lit.group(1))
            continue
        ref = re.match(r"(\w+)\s*->", tok) or re.fullmatch(r"(\w+)", tok)
        if ref and ref.group(1) in vars:
            rhs.append(vars[ref.group(1)])
            continue
        return None  # unresolved operand -> not cleanly expressible

    expr = re.sub(r";\s*$", "", "".join(rhs))
    expr = re.sub(r"\s+", " ", expr).strip()
    if not expr:
        return None
    return out_name + " = " + expr


def scan_deprecated():
    """Return the set of node class names tagged with the T_DEPRECATED attribute."""
    names = set()
    for pattern in NODE_HEADER_GLOBS:
        for path in glob.glob(os.path.join(ROOT, pattern)):
            with open(path, encoding="utf-8") as f:
                for line in f:
                    s = line.strip()
                    if not s.startswith("class") or "T_DEPRECATED" not in s:
                        continue
                    # Class name is the identifier just before the base-class colon,
                    # regardless of where T_DEPRECATED/T_DLLCLASS sit.
                    m = re.search(r"(\w+)\s*:", s)
                    if m and m.group(1) not in ("class",):
                        names.add(m.group(1))
    return names


def cpp_escape_raw(text):
    # Raw string with a unique delimiter; guard against the (unlikely) delimiter
    # appearing in the source.
    assert (")" + DELIM + '"') not in text, "raw-string delimiter collision"
    return text


def main():
    with open(SRC, encoding="utf-8") as f:
        lines = f.readlines()
    text = "".join(lines)

    funcs = extract_functions(lines)
    regs = extract_registrations(text)

    captured = {}
    summaries = {}
    for node_type, fn in sorted(regs.items()):
        if fn in funcs:
            captured[node_type] = funcs[fn]
            s = summarize(funcs[fn])
            if s:
                summaries[node_type] = s

    missing = sorted(set(regs) - set(captured))
    deprecated = sorted(scan_deprecated())

    out = []
    out.append("/*")
    out.append(" * TRAKTOR")
    out.append(" * Copyright (c) 2026 Anders Pistol.")
    out.append(" *")
    out.append(" * This Source Code Form is subject to the terms of the Mozilla Public")
    out.append(" * License, v. 2.0. If a copy of the MPL was not distributed with this")
    out.append(" * file, You can obtain one at https://mozilla.org/MPL/2.0/.")
    out.append(" */")
    out.append("// GENERATED FILE - DO NOT EDIT.")
    out.append("// Produced by scripts/generate-node-emitter-capture.py from")
    out.append("// code/Render/Editor/Glsl/GlslEmitter.cpp and the shader node headers.")
    out.append('#include "MCP/Server/Editor/NodeEmitterCapture.h"')
    out.append("")
    out.append("namespace traktor::mcp")
    out.append("{")
    out.append("")
    out.append("const std::map< std::wstring, std::wstring >& nodeEmitterCapture()")
    out.append("{")
    out.append("\tstatic const std::map< std::wstring, std::wstring > s_capture = {")
    for node_type in sorted(captured):
        body = cpp_escape_raw(captured[node_type])
        out.append('\t\t{{ L"{0}", LR"{1}({2}){1}" }},'.format(node_type, DELIM, body))
    out.append("\t};")
    out.append("\treturn s_capture;")
    out.append("}")
    out.append("")
    out.append("const std::map< std::wstring, std::wstring >& nodeEmitterSummary()")
    out.append("{")
    out.append("\tstatic const std::map< std::wstring, std::wstring > s_summary = {")
    for node_type in sorted(summaries):
        text = summaries[node_type].replace("\\", "\\\\").replace('"', '\\"')
        out.append('\t\t{{ L"{0}", L"{1}" }},'.format(node_type, text))
    out.append("\t};")
    out.append("\treturn s_summary;")
    out.append("}")
    out.append("")
    out.append("const std::set< std::wstring >& nodeDeprecatedTypes()")
    out.append("{")
    out.append("\tstatic const std::set< std::wstring > s_deprecated = {")
    for node_type in deprecated:
        out.append('\t\tL"{0}",'.format(node_type))
    out.append("\t};")
    out.append("\treturn s_deprecated;")
    out.append("}")
    out.append("")
    out.append("}")
    out.append("")

    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(out))

    print("captured {0} node types ({1} with one-line summary, {2} deprecated) -> {3}".format(len(captured), len(summaries), len(deprecated), os.path.relpath(OUT, ROOT)))
    if deprecated:
        print("deprecated: {0}".format(", ".join(deprecated)))
    if missing:
        print("registered but no function body found ({0}): {1}".format(len(missing), ", ".join(missing)))


if __name__ == "__main__":
    sys.exit(main())
