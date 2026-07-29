#!/usr/bin/env python3
"""Expand milmon library includes into the modules used by a C++ solution."""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


INCLUDE_RE = re.compile(
    r"^(?P<indent>[ \t]*)#[ \t]*include[ \t]*[<\"](?P<header>[^>\"]+)[>\"]"
    r"[^\r\n]*(?P<newline>\r?\n|$)",
    re.MULTILINE,
)
STANDARD_INCLUDE_RE = re.compile(
    r"^[ \t]*#[ \t]*include[ \t]*<(?P<header>[^>]+)>[^\r\n]*(?:\r?\n|$)",
    re.MULTILINE,
)
REQUIRE_RE = re.compile(
    r"//[ \t]*milmon[ \t]*:[ \t]*require[ \t]+(?P<modules>[^\r\n]+)",
    re.IGNORECASE,
)
IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
RAW_STRING_RE = re.compile(r'(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(')
QUOTED_LITERAL_RE = re.compile(r'(?:u8|u|U|L)?(?P<quote>["\'])')


class BundleError(RuntimeError):
    """An input or library configuration error."""


@dataclass(frozen=True)
class Module:
    name: str
    header: Path
    symbols: tuple[str, ...]
    depends: tuple[str, ...]


@dataclass(frozen=True)
class Library:
    root: Path
    umbrella_headers: frozenset[str]
    modules: dict[str, Module]
    direct_headers: dict[str, str]


def load_library(manifest_path: Path) -> Library:
    try:
        data = json.loads(manifest_path.read_text(encoding="utf-8"))
    except OSError as error:
        raise BundleError(f"cannot read manifest {manifest_path}: {error}") from error
    except json.JSONDecodeError as error:
        raise BundleError(f"invalid JSON in {manifest_path}: {error}") from error

    if data.get("version") != 1:
        raise BundleError("the manifest must have version 1")
    raw_modules = data.get("modules")
    if not isinstance(raw_modules, dict) or not raw_modules:
        raise BundleError("the manifest must contain a non-empty 'modules' object")

    root = manifest_path.resolve().parent
    modules: dict[str, Module] = {}
    direct_headers: dict[str, str] = {}
    for name, raw_module in raw_modules.items():
        if not isinstance(name, str) or not IDENTIFIER_RE.fullmatch(name):
            raise BundleError(f"invalid module name: {name!r}")
        if not isinstance(raw_module, dict):
            raise BundleError(f"module {name!r} must be an object")

        header_value = raw_module.get("header")
        symbols_value = raw_module.get("symbols")
        depends_value = raw_module.get("depends", [])
        if not isinstance(header_value, str):
            raise BundleError(f"module {name!r} has no valid header")
        if not isinstance(symbols_value, list) or not all(
            isinstance(symbol, str) and IDENTIFIER_RE.fullmatch(symbol)
            for symbol in symbols_value
        ):
            raise BundleError(f"module {name!r} has invalid symbols")
        if not isinstance(depends_value, list) or not all(
            isinstance(dependency, str) for dependency in depends_value
        ):
            raise BundleError(f"module {name!r} has invalid dependencies")

        relative_header = Path(header_value)
        module = Module(
            name=name,
            header=root / relative_header,
            symbols=tuple(symbols_value),
            depends=tuple(depends_value),
        )
        modules[name] = module

        normalized = relative_header.as_posix()
        direct_headers[normalized] = name
        if normalized.startswith("include/"):
            direct_headers[normalized[len("include/") :]] = name

    for module in modules.values():
        missing = [dependency for dependency in module.depends if dependency not in modules]
        if missing:
            raise BundleError(
                f"module {module.name!r} depends on unknown module(s): {', '.join(missing)}"
            )
        if not module.header.is_file():
            raise BundleError(f"header for module {module.name!r} does not exist: {module.header}")

    umbrella_value = data.get("umbrella_headers", [])
    if not isinstance(umbrella_value, list) or not all(
        isinstance(header, str) for header in umbrella_value
    ):
        raise BundleError("'umbrella_headers' must be a list of strings")
    return Library(
        root=root,
        umbrella_headers=frozenset(umbrella_value),
        modules=modules,
        direct_headers=direct_headers,
    )


def blank_range(characters: list[str], start: int, end: int) -> None:
    for index in range(start, end):
        if characters[index] not in "\r\n":
            characters[index] = " "


def code_without_comments_and_literals(source: str) -> str:
    """Blank comments and literals while preserving positions and line endings."""
    characters = list(source)
    index = 0
    length = len(source)
    while index < length:
        if source.startswith("//", index):
            end = source.find("\n", index + 2)
            end = length if end == -1 else end
            blank_range(characters, index, end)
            index = end
            continue
        if source.startswith("/*", index):
            closing = source.find("*/", index + 2)
            end = length if closing == -1 else closing + 2
            blank_range(characters, index, end)
            index = end
            continue

        raw_match = RAW_STRING_RE.match(source, index)
        if raw_match:
            terminator = ")" + raw_match.group("delimiter") + '"'
            closing = source.find(terminator, raw_match.end())
            end = length if closing == -1 else closing + len(terminator)
            blank_range(characters, index, end)
            index = end
            continue

        quoted_match = QUOTED_LITERAL_RE.match(source, index)
        if quoted_match:
            quote = quoted_match.group("quote")
            cursor = quoted_match.end()
            while cursor < length:
                if source[cursor] == "\\":
                    cursor += 2
                    continue
                cursor += 1
                if source[cursor - 1] == quote:
                    break
            blank_range(characters, index, min(cursor, length))
            index = cursor
            continue

        index += 1
    return "".join(characters)


def active_include_starts(source: str, pattern: re.Pattern[str]) -> set[int]:
    code = code_without_comments_and_literals(source)
    return {
        match.start()
        for match in pattern.finditer(source)
        if "#" in code[match.start() : match.start("header")]
    }


def split_module_names(values: Iterable[str]) -> set[str]:
    names: set[str] = set()
    for value in values:
        names.update(part for part in re.split(r"[\s,]+", value.strip()) if part)
    return names


def has_managed_include(source: str, library: Library) -> bool:
    active = active_include_starts(source, INCLUDE_RE)
    return any(
        match.start() in active
        and (
            match.group("header") in library.umbrella_headers
            or match.group("header") in library.direct_headers
        )
        for match in INCLUDE_RE.finditer(source)
    )


def resolve_modules(
    source: str, library: Library, required_by_cli: Sequence[str]
) -> tuple[list[Module], dict[str, set[str]]]:
    active = active_include_starts(source, INCLUDE_RE)
    includes = [match for match in INCLUDE_RE.finditer(source) if match.start() in active]
    managed_includes = [
        match
        for match in includes
        if match.group("header") in library.umbrella_headers
        or match.group("header") in library.direct_headers
    ]
    if not managed_includes:
        return [], {}

    reasons: dict[str, set[str]] = {}

    def select(name: str, reason: str) -> None:
        if name not in library.modules:
            raise BundleError(f"unknown module {name!r}")
        reasons.setdefault(name, set()).add(reason)

    has_umbrella = False
    for match in managed_includes:
        header = match.group("header")
        if header in library.umbrella_headers:
            has_umbrella = True
        else:
            select(library.direct_headers[header], f"included as {header}")

    directive_names = split_module_names(
        match.group("modules") for match in REQUIRE_RE.finditer(source)
    )
    cli_names = split_module_names(required_by_cli)
    for name in sorted(directive_names):
        select(name, "source directive")
    for name in sorted(cli_names):
        select(name, "command line")

    if has_umbrella:
        code = code_without_comments_and_literals(source)
        identifiers = set(IDENTIFIER_RE.findall(code))
        for module in library.modules.values():
            matched = identifiers.intersection(module.symbols)
            if matched:
                select(module.name, "symbol " + ", ".join(sorted(matched)))

    # Materialize the full dependency closure before ordering. This keeps modules
    # such as global type aliases at their manifest position while DFS below still
    # guarantees that every dependency precedes its consumer.
    pending = list(reasons)
    expanded: set[str] = set()
    while pending:
        name = pending.pop()
        if name in expanded:
            continue
        expanded.add(name)
        for dependency in library.modules[name].depends:
            reasons.setdefault(dependency, set()).add(f"dependency of {name}")
            pending.append(dependency)

    ordered: list[Module] = []
    visited: set[str] = set()
    visiting: set[str] = set()

    def visit(name: str) -> None:
        if name in visited:
            return
        if name in visiting:
            raise BundleError(f"dependency cycle involving module {name!r}")
        visiting.add(name)
        module = library.modules[name]
        for dependency in module.depends:
            reasons.setdefault(dependency, set()).add(f"dependency of {name}")
            visit(dependency)
        visiting.remove(name)
        visited.add(name)
        ordered.append(module)

    # Manifest order makes generated submissions stable across runs.
    for name in library.modules:
        if name in reasons:
            visit(name)
    return ordered, reasons


def remove_standard_includes(
    source: str, excluded: frozenset[str] = frozenset()
) -> tuple[str, list[str]]:
    headers: list[str] = []
    active = active_include_starts(source, STANDARD_INCLUDE_RE)

    def replace(match: re.Match[str]) -> str:
        if match.start() not in active:
            return match.group(0)
        header = match.group("header").strip()
        if header in excluded:
            return match.group(0)
        headers.append(header)
        return ""

    return STANDARD_INCLUDE_RE.sub(replace, source), headers


def header_body(module: Module, library: Library) -> tuple[str, list[str]]:
    try:
        text = module.header.read_text(encoding="utf-8")
    except OSError as error:
        raise BundleError(f"cannot read {module.header}: {error}") from error

    lines: list[str] = []
    for line in text.splitlines(keepends=True):
        if re.fullmatch(r"[ \t]*#[ \t]*pragma[ \t]+once[ \t]*(?:\r?\n)?", line):
            continue
        include_match = INCLUDE_RE.fullmatch(line)
        if include_match:
            header = include_match.group("header")
            if header in library.umbrella_headers or header in library.direct_headers:
                continue
        lines.append(line)
    body, headers = remove_standard_includes("".join(lines))
    return body.strip(), headers


def module_label(module: Module, library: Library) -> str:
    try:
        path = module.header.relative_to(library.root).as_posix()
    except ValueError:
        return module.header.name
    for prefix in ("include/milmon/", "include/"):
        if path.startswith(prefix):
            return path[len(prefix) :]
    return path


def render_bundle(
    modules: Sequence[Module], library: Library
) -> tuple[str, list[str]]:
    if not modules:
        return "", []

    sections: list[str] = []
    headers: list[str] = []
    for module in modules:
        body, module_headers = header_body(module, library)
        headers.extend(module_headers)
        sections.append(f"// milmon-lib/{module_label(module, library)}\n{body}\n")
    sections.append("// milmon-lib ends\n")
    return "\n".join(sections), headers


def replace_managed_includes(source: str, library: Library, bundle: str) -> str:
    inserted = False
    active = active_include_starts(source, INCLUDE_RE)

    def replace(match: re.Match[str]) -> str:
        nonlocal inserted
        if match.start() not in active:
            return match.group(0)
        header = match.group("header")
        if header not in library.umbrella_headers and header not in library.direct_headers:
            return match.group(0)
        if inserted:
            return ""
        inserted = True
        return bundle

    return INCLUDE_RE.sub(replace, source)


def render_output(source: str, modules: Sequence[Module], library: Library) -> str:
    excluded = library.umbrella_headers | frozenset(library.direct_headers)
    source, source_headers = remove_standard_includes(source, excluded)
    bundle, module_headers = render_bundle(modules, library)
    body = replace_managed_includes(source, library, bundle).lstrip("\r\n")

    headers: list[str] = []
    seen: set[str] = set()
    for header in source_headers + module_headers:
        if header not in seen:
            seen.add(header)
            headers.append(f"#include <{header}>")
    if not headers:
        return body
    return "\n".join(headers) + "\n\n" + body


def build_argument_parser(default_manifest: Path) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="replace milmon C++ includes with only the modules used by a solution"
    )
    parser.add_argument("source", nargs="?", type=Path, help="input C++ source file")
    parser.add_argument("-o", "--output", type=Path, help="output file (default: stdout)")
    parser.add_argument(
        "--manifest", type=Path, default=default_manifest, help="path to library.json"
    )
    parser.add_argument(
        "--require",
        action="append",
        default=[],
        metavar="MODULE",
        help="force a module to be included; may be repeated or comma-separated",
    )
    parser.add_argument("--list", action="store_true", help="list available modules and exit")
    parser.add_argument(
        "--explain", action="store_true", help="print why every module was selected"
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    default_manifest = Path(__file__).resolve().parents[1] / "library.json"
    parser = build_argument_parser(default_manifest)
    arguments = parser.parse_args(argv)

    try:
        library = load_library(arguments.manifest)
        if arguments.list:
            for module in library.modules.values():
                print(f"{module.name}: {', '.join(module.symbols)}")
            return 0
        if arguments.source is None:
            parser.error("source is required unless --list is used")

        source_path = arguments.source.resolve()
        if arguments.output is not None and arguments.output.resolve() == source_path:
            raise BundleError("refusing to overwrite the input file; choose another output path")
        try:
            source = source_path.read_text(encoding="utf-8")
        except OSError as error:
            raise BundleError(f"cannot read {source_path}: {error}") from error

        managed = has_managed_include(source, library)
        modules, reasons = resolve_modules(source, library, arguments.require)
        output = render_output(source, modules, library) if managed else source

        if arguments.output is None:
            sys.stdout.write(output)
        else:
            try:
                arguments.output.write_text(output, encoding="utf-8")
            except OSError as error:
                raise BundleError(f"cannot write {arguments.output}: {error}") from error

        selected = ", ".join(module.name for module in modules) or "(none)"
        print(f"milmon-bundle: selected {selected}", file=sys.stderr)
        if arguments.explain:
            for module in modules:
                print(
                    f"  {module.name}: {'; '.join(sorted(reasons[module.name]))}",
                    file=sys.stderr,
                )
        return 0
    except BundleError as error:
        print(f"milmon-bundle: error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
