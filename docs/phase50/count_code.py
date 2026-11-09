import os

extensions = {
    ".cpp": "C++ Source",
    ".hpp": "C++ Header",
    ".h": "C/C++ Header",
    ".lua": "Lua Script",
    ".go": "Go Source",
    ".sql": "SQL Migration",
    ".py": "Python Tool",
    ".json": "JSON Data/Config",
    ".yaml": "YAML Config",
    ".cmake": "CMake Script",
    ".txt": "CMake/Text"
}

stats = {k: {"files": 0, "lines": 0} for k in extensions.values()}

for root, dirs, files in os.walk("."):
    if any(b in root for b in ["\\build", "/build", "\\.git", "/.git", "\\.gemini", "/.gemini"]):
        continue
    for f in files:
        ext = os.path.splitext(f)[1].lower()
        if ext in extensions:
            desc = extensions[ext]
            p = os.path.join(root, f)
            stats[desc]["files"] += 1
            try:
                with open(p, "r", encoding="utf-8", errors="ignore") as fp:
                    stats[desc]["lines"] += sum(1 for _ in fp)
            except Exception:
                pass

print("=== WILDBOUND CODEBASE REPOSITORY COMPOSITION ===")
total_f = sum(v["files"] for v in stats.values())
total_l = sum(v["lines"] for v in stats.values())
for k, v in sorted(stats.items(), key=lambda x: x[1]["lines"], reverse=True):
    print(f"{k:15}: {v['files']:5} files, {v['lines']:8} lines")
print(f"{'TOTAL':15}: {total_f:5} files, {total_l:8} lines")

