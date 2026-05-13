#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BENCHMARK_DIR="$SCRIPT_DIR/nativejson-benchmark"

echo "=== Setting up nativejson-benchmark ==="

if [ ! -d "$BENCHMARK_DIR" ]; then
    git clone --depth=1 https://github.com/miloyip/nativejson-benchmark.git "$BENCHMARK_DIR"
fi

cd "$BENCHMARK_DIR"

# Update submodules (nlohmann/json, etc.)
git submodule update --init --depth=1 || true

# Copy our library into the benchmark's thirdparty tree
mkdir -p thirdparty/jason
cp "$PROJECT_ROOT/src/jason.h" thirdparty/jason/jason.h

# Copy our test wrapper into the benchmark's test tree
cp "$SCRIPT_DIR/jasontest.cpp" src/tests/jasontest.cpp

# Keep only jason, nlohmann, and RapidJSON (required as reference) wrappers.
# All others are removed to speed up the build.
cd src/tests
for f in *.cpp; do
    if [ "$f" != "nlohmanntest.cpp" ] && [ "$f" != "jasontest.cpp" ] && [ "$f" != "rapidjsontest.cpp" ]; then
        rm -f "$f"
    fi
done
cd "$BENCHMARK_DIR"

# Ensure premake5 is available
if ! command -v premake5 &> /dev/null; then
    if [ ! -f build/premake5 ]; then
        echo "Downloading premake5..."
        if [[ "$OSTYPE" == "darwin"* ]]; then
            PREMAKE_URL="https://github.com/premake/premake-core/releases/download/v5.0.0-alpha16/premake-5.0.0-alpha16-macosx.tar.gz"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            PREMAKE_URL="https://github.com/premake/premake-core/releases/download/v5.0.0-alpha16/premake-5.0.0-alpha16-linux.tar.gz"
        else
            echo "Unsupported OS for automatic premake5 download. Please install premake5 manually."
            exit 1
        fi
        curl -sL "$PREMAKE_URL" | tar xz -C build
        chmod +x build/premake5
    fi
fi
# Always ensure local premake5 is in PATH (prefer downloaded version)
export PATH="$BENCHMARK_DIR/build:$PATH"

# Patch premake5.lua for C++17
if grep -q '\-std=c++14' build/premake5.lua; then
    echo "Patching build/premake5.lua for C++17..."
    sed -i.bak 's/-std=c++14/-std=c++17/g' build/premake5.lua
    rm -f build/premake5.lua.bak
fi

# Remove deprecated ExtraWarnings flag
if grep -q 'ExtraWarnings' build/premake5.lua; then
    echo "Patching build/premake5.lua to remove ExtraWarnings..."
    sed -i.bak '/flags { "ExtraWarnings" }/d' build/premake5.lua
    rm -f build/premake5.lua.bak
fi

# Remove hardcoded ULib linkoption
if grep -q 'libulib.a' build/premake5.lua; then
    echo "Patching build/premake5.lua to remove ULib linkoption..."
    sed -i.bak '/libulib.a/d' build/premake5.lua
    rm -f build/premake5.lua.bak
fi

# Regenerate makefiles
cd build
./premake.sh
cd ..

# Build the benchmark binary only (avoid PHP dependency for HTML generation)
echo "=== Building benchmark binary (this may take a while) ==="
make CC=clang CXX=clang++ CXXFLAGS="-stdlib=libc++" LDFLAGS="-stdlib=libc++" bin/nativejson_release_x64_gmake CONFIG=release_x64

# Run the benchmark binary from bin/ so relative data paths resolve correctly
echo "=== Running benchmark ==="
cd bin
./nativejson_release_x64_gmake
cd ..

echo "=== Generating comparison report ==="
cd "$PROJECT_ROOT"
python3 "$SCRIPT_DIR/benchmark_report.py"

echo ""
echo "=== Benchmark complete ==="
echo "Raw results: $BENCHMARK_DIR/result/"
echo "Comparison report: $SCRIPT_DIR/benchmark_report.md"
