# Hydruino Tests

Run the host-testable core without Arduino hardware:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The host suite covers elapsed-time rollover handling, crop phase selection, feeding cadence, binary input stability, signed actuator direction, balancing behavior, timed dosing estimates, and append-only binary record migration helpers.

When Python is available, CTest also runs the source validator. It checks the crop database and several framework regressions that are easy to reintroduce during refactors.

Source checks can also be run directly:

```sh
python3 tests/validate_source.py
```

Development Arduino sketches are included for tasks that are useful on actual hardware or with the Arduino build environment:

* `CropLibExportToCPP` exports the built-in crop library into C++ data.
* `EnumConversionTests` checks enum string conversions.
* `EnumTrieExportToCPP` exports the compact enum decoder tree.
* `JSONExportTests` exercises JSON serialization paths.
