### PR Description

#### Goal
The primary goal of this PR is to significantly improve the test coverage for the Phlex Python plugin and resolve several discovered issues related to memory management, type conversion, and core graph connectivity safety. It also modernizes the build system configuration and coverage tooling.

#### Key Changes and Rationale

**1. Introduction of `phlex.Variant` Helper**
- **Changes**: Added a new Python class `Variant` in `plugins/python/python/phlex/__init__.py`.
- **Motivation**: Python algorithms are often generic, but Phlex registration requires specific C++ signatures. Additionally, Python parameter names in a function's `__annotations__` might not match the positional labels defined in the Phlex configuration.
- **Solution**: The `Variant` class allows developers to wrap a callable and associate it with custom annotations and a specific name. This enables the same Python function to be registered multiple times with different types without modifying the original function.
- **Rationale**: This decouples the algorithm implementation from the registration metadata. The C++ bridge was also updated to "unwrap" these objects, retrieving the underlying `phlex_callable` for performance while still using the provided annotations for type mapping.

**2. Robustness Improvements in `modulewrap.cpp`**
- **Reference Counting**: Added missing `Py_DECREF` calls in the error paths of `parse_args`, `md_transform`, and `md_observe`. This prevents Python object leaks when registration fails.
- **Sequence Handling**: Replaced manual sequence checks with `PySequence_Fast`. This allows Phlex to accept any Python sequence (lists, tuples, etc.) for input/output labels.
- **Type Matching & NumPy Fixes**:
    - Corrected a typo where double-precision NumPy arrays were matched against `double64]]` instead of `float64]]`.
    - Updated type matching logic to use fixed-length substring comparisons, supporting PEP 604 union types (e.g., `ndarray | list`).
- **List-based Type Converters**: Added explicit support for `list[int]`, `list[float]`, and `list[double]`.
    - **Rationale**: Provides a fallback for users not using NumPy or for cases where standard Python lists are more appropriate.

**3. Core Connectivity & Safety**
- **Changes**: Added null-pointer checks in `phlex/core/edge_maker.cpp` and `phlex/core/edge_maker.hpp`.
- **Motivation**: Misconfigurations in the Jsonnet graph could lead to null ports during edge creation, resulting in segmentation faults.
- **Solution**: The code now throws a `std::runtime_error` with a descriptive message identifying the offending nodes/ports.

**4. Build System & Tooling Modernization**
- **CMake Optimizations**:
    - Introduced `DART_TESTING_TIMEOUT` and `CTEST_TEST_TIMEOUT` to prevent long stalls in CI environments.
    - Refined GCC warning suppressions for versions 14+ to reduce noise from false-positive array-bounds and string-overflow warnings.
    - Standardized Sanatizer (ASan/TSan) compile options for better compatibility.
- **Coverage Tooling**: Updated `scripts/coverage.sh` and its documentation to support both Clang (LLVM source-based) and GCC (gcov) presets, enabling both high-fidelity local reports and CI-compatible XML outputs.
- **Documentation**: Added `plugins/python/README.md` (architecture guide) and `.github/copilot-instructions.md` (project context for AI assistants).

#### New Tests

This PR adds a comprehensive suite of tests in `test/python/` to ensure long-term stability:

- **`unit_test_variant.py`**: Dedicated unit tests for the `Variant` class logic.
- **`test_types.py` & `test_coverage.py`**: Exercise various scalar, vector, and list-based type conversions to ensure data integrity.
- **`test_callbacks.py`**: Targets edge cases in `modulewrap.cpp`, such as 3-argument callbacks and Python exception propagation.
- **Error Path Verification**: Added several tests (`pybadbool.jsonnet`, `pybadint.jsonnet`, `pymismatch.jsonnet`, etc.) that verify Phlex correctly catches and reports type and signature mismatches.
- **Coverage Integration**: Updated `test/python/CMakeLists.txt` to support `pytest-cov`, enabling integrated Python coverage reporting.
