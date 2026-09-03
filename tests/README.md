# Hydruino Tests

Run the host-testable core without Arduino hardware:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The host suite covers elapsed-time rollover handling, crop phase selection, feeding cadence, binary input stability, signed actuator direction, balancing behavior, timed dosing estimates, and append-only binary record migration helpers. It also builds the full Hydruino source and exercises controller initialization, object registration and reconstruction, factory-created hardware objects, activation and attachments, measurement conversion, and user calibration and additive lifecycle. Serialization coverage round-trips system, calibration, actuator, binary sensor, and trigger data through ArduinoJson and exercises JSON-based data allocation.

Development Arduino sketches are included for tasks that are useful on actual hardware or with the Arduino build environment:

* `CropLibExportToCPP` exports the built-in crop library into C++ data.
* `EnumConversionTests` checks enum string conversions.
* `EnumTrieExportToCPP` exports the compact enum decoder tree.
* `JSONExportTests` exercises JSON serialization paths.
