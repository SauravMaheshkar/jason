#!/usr/bin/env python3
"""Generate a comparison report from nativejson-benchmark results.

Reads the latest performance CSV and conformance markdown files from
tests/nativejson-benchmark/result/ and writes tests/benchmark_report.md.
"""

import csv
import glob
import os
import re
import sys
from pathlib import Path


def find_latest_csv(result_dir: Path) -> Path:
    """Return the most recently modified performance_*.csv file."""
    csv_files = list(result_dir.glob("performance_*.csv"))
    if not csv_files:
        print("No performance CSV found in", result_dir)
        sys.exit(1)
    return max(csv_files, key=lambda p: p.stat().st_mtime)


def parse_conformance(md_path: Path) -> dict:
    """Extract conformance scores from a markdown file."""
    scores = {}
    text = md_path.read_text(encoding="utf-8")
    # Look for lines like "Summary: X of Y are correct."
    for match in re.finditer(r"Summary:\s*(\d+)\s+of\s+(\d+)\s+are correct", text):
        correct = int(match.group(1))
        total = int(match.group(2))
        # Determine which section this belongs to by looking at the nearest preceding heading
        preceding = text[:match.start()]
        section = "Unknown"
        for heading in re.finditer(r"##\s*(\d+)\.\s*(.+)", preceding):
            section = heading.group(2).strip()
        scores[section] = (correct, total)
    return scores


def parse_performance(csv_path: Path) -> dict:
    """Extract performance data into a nested dict: lib -> test_type -> file -> metrics."""
    data = {}
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            lib = row["Library"]
            test_type = row["Type"]
            filename = row["Filename"]
            if lib not in data:
                data[lib] = {}
            if test_type not in data[lib]:
                data[lib][test_type] = {}
            data[lib][test_type][filename] = {
                "time_ms": float(row["Time (ms)"]),
                "memory": int(row["Memory (byte)"]),
                "memory_peak": int(row["MemoryPeak (byte)"]),
                "alloc_count": int(row["AllocCount"]),
                "file_size": int(row["FileSize (byte)"]),
            }
    return data


def format_unified_conformance_table(libs_scores: dict, libs: list) -> str:
    """Build a single markdown conformance table with one column per library."""
    # Collect all sections in order of first appearance across libraries
    all_sections = []
    seen = set()
    for lib in libs:
        if lib not in libs_scores:
            continue
        for section in libs_scores[lib].keys():
            if section not in seen:
                seen.add(section)
                all_sections.append(section)

    lines = ["| Test | " + " | ".join(libs) + " |"]
    lines.append("|------|" + "|".join(["------"] * len(libs)) + "|")

    # Per-library totals for overall row
    totals = {lib: (0, 0) for lib in libs}

    for section in all_sections:
        row = [section]
        for lib in libs:
            if lib in libs_scores and section in libs_scores[lib]:
                correct, total = libs_scores[lib][section]
                if total == 0:
                    row.append("-")
                else:
                    pct = correct / total * 100
                    row.append(f"{correct}/{total} ({pct:.0f}%)")
                totals[lib] = (totals[lib][0] + correct, totals[lib][1] + total)
            else:
                row.append("-")
        lines.append("| " + " | ".join(row) + " |")

    # Overall row
    overall_row = ["**Overall**"]
    for lib in libs:
        correct, total = totals[lib]
        if total == 0:
            overall_row.append("**-**")
        else:
            pct = correct / total * 100
            overall_row.append(f"**{correct}/{total} ({pct:.0f}%)**")
    lines.append("| " + " | ".join(overall_row) + " |")
    lines.append("")

    return "\n".join(lines)


def format_performance_table(perf: dict, libs: list) -> str:
    """Build a markdown table for performance metrics."""
    test_types = [
        ("1. Parse", "Parse"),
        ("4. Statistics", "Statistics"),
        ("7. Code size", "Code size"),
    ]
    files = ["canada.json", "citm_catalog.json", "twitter.json", "jsonstat"]

    lines = ["| Test | File | " + " | ".join(libs) + " |"]
    lines.append("|------|------|" + "|".join(["------"] * len(libs)) + "|")

    for csv_type, display_type in test_types:
        for file in files:
            row = [display_type, file]
            has_any = False
            for lib in libs:
                if lib in perf and csv_type in perf[lib] and file in perf[lib][csv_type]:
                    m = perf[lib][csv_type][file]
                    if csv_type == "7. Code size":
                        val = f"{m['file_size']:,} B"
                    else:
                        val = f"{m['time_ms']:.3f} ms"
                    row.append(val)
                    has_any = True
                else:
                    row.append("N/A")
            if has_any:
                lines.append("| " + " | ".join(row) + " |")
    lines.append("")
    return "\n".join(lines)


def main():
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    result_dir = project_root / "tests" / "nativejson-benchmark" / "result"

    if not result_dir.exists():
        print("Results directory not found:", result_dir)
        sys.exit(1)

    csv_path = find_latest_csv(result_dir)
    perf = parse_performance(csv_path)

    # Determine library names from the CSV
    all_libs = list(perf.keys())
    jason_name = next((l for l in all_libs if "jason" in l.lower()), None)
    nlohmann_name = next((l for l in all_libs if "nlohmann" in l.lower()), None)

    libs_in_order = []
    if jason_name:
        libs_in_order.append(jason_name)
    if nlohmann_name:
        libs_in_order.append(nlohmann_name)
    for l in all_libs:
        if l not in libs_in_order:
            libs_in_order.append(l)

    # Parse conformance
    libs_scores = {}
    for lib in libs_in_order:
        md_path = result_dir / f"conformance_{lib}.md"
        if md_path.exists():
            libs_scores[lib] = parse_conformance(md_path)

    # Build report
    report_lines = [
        "# Benchmark Report: jason vs nlohmann/json",
        "",
        "Generated from [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark).",
        "",
        "## Conformance",
        "",
    ]
    report_lines.append(format_unified_conformance_table(libs_scores, libs_in_order))

    report_lines.extend([
        "## Performance",
        "",
        format_performance_table(perf, libs_in_order),
    ])

    # Add analysis note about jason
    if jason_name and nlohmann_name:
        jason_has_parse = any(
            t.startswith("1. Parse") and f != "jsonstat"
            for t, files in perf.get(jason_name, {}).items()
            for f in files
        )
        if not jason_has_parse:
            report_lines.extend([
                "### Note on jason Performance",
                "",
                "**jason does not produce parse performance numbers**",
                "for the standard benchmark test files",
                "(`canada.json`, `citm_catalog.json`, `twitter.json`).",
                "The current jason implementation is intentionally minimal and lacks support for:",
                "",
                "- Negative numbers",
                "- Scientific notation (e.g. `1e10`)",
                "- Unicode escapes (`\\uXXXX`)",
                "- Many backslash escape sequences",
                "",
                "Therefore the benchmark framework marks these capabilities as \"Not support\".",
                "",
            ])

    # Code-size focused comparison
    report_lines.extend([
        "### Code Size Comparison",
        "",
    ])
    for lib in libs_in_order:
        if lib in perf and "7. Code size" in perf[lib] and "jsonstat" in perf[lib]["7. Code size"]:
            size = perf[lib]["7. Code size"]["jsonstat"]["file_size"]
            report_lines.append(f"- **{lib}**: {size:,} bytes")
    report_lines.append("")

    report_path = script_dir / "benchmark_report.md"
    report_path.write_text("\n".join(report_lines), encoding="utf-8")
    print(f"Report written to {report_path}")


if __name__ == "__main__":
    main()
