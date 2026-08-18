#!/usr/bin/env python3
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def parse_range(value, count):
    values = [float(value)] if isinstance(value, (int, float)) else [float(part) for part in str(value).split(",")]
    if len(values) == 1:
        values *= count
    require(len(values) == count, f"Expected {count} values, got {value!r}")
    return values


def validate_crop_database():
    text = (SRC / "HydroCropsLibrary.cpp").read_text()
    matches = re.findall(r'static const char flashStr_(\w+)\[\] PROGMEM = \{"((?:\\.|[^"])*)"\};', text)
    require(len(matches) == 77, f"Expected 77 built-in crops, found {len(matches)}")

    required = {
        "type", "id", "cropName", "totalGrowWeeks", "phaseDurationWeeks", "dailyLightHours",
        "phRange", "tdsRange", "nightlyFeedRate", "waterTempRange", "airTempRange", "co2Levels"
    }

    for symbol, escaped in matches:
        crop = json.loads(bytes(escaped, "utf8").decode("unicode_escape"))
        require(crop["id"] == symbol, f"Crop symbol/id mismatch for {symbol}")
        require(required.issubset(crop), f"{symbol} is missing fields: {sorted(required - set(crop))}")

        total_weeks = int(crop["totalGrowWeeks"])
        phases = [int(value) for value in str(crop["phaseDurationWeeks"]).split(",")]
        lights = [int(value) for value in str(crop["dailyLightHours"]).split(",")]
        ph = parse_range(crop["phRange"], 2)
        ec = parse_range(crop["tdsRange"], 2)
        water_temp = parse_range(crop["waterTempRange"], 2)
        air_temp = parse_range(crop["airTempRange"], 2)
        co2 = parse_range(crop["co2Levels"], 2)

        require(1 <= total_weeks <= 127, f"{symbol}: totalGrowWeeks out of supported range")
        require(len(phases) == 3 and sum(phases) == total_weeks, f"{symbol}: phase weeks do not sum to total")
        require(len(lights) == 3 and all(1 <= hours <= 24 for hours in lights), f"{symbol}: invalid light hours")
        require(3.5 <= ph[0] <= ph[1] <= 9.0, f"{symbol}: invalid pH range {ph}")
        require(0.2 <= ec[0] <= ec[1] <= 4.0, f"{symbol}: invalid EC range {ec}")
        require(10 <= water_temp[0] <= water_temp[1] <= 30, f"{symbol}: invalid water temperature {water_temp}")
        require(10 <= air_temp[0] <= air_temp[1] <= 35, f"{symbol}: invalid air temperature {air_temp}")
        require(300 <= co2[0] <= 1500 and 300 <= co2[1] <= 1500, f"{symbol}: invalid CO2 levels {co2}")
        require(0 < float(crop["nightlyFeedRate"]) <= 1.5, f"{symbol}: invalid nightly feed rate")


def validate_factories():
    files = ["HydroObject.cpp", "HydroActuators.cpp", "HydroSensors.cpp", "HydroCrops.cpp", "HydroReservoirs.cpp", "HydroRails.cpp"]
    for filename in files:
        text = (SRC / filename).read_text()
        require("dataIn && !isValidType(dataIn->id.object.idType)" in text,
                f"{filename}: factory validity guard is missing or inverted")


def validate_scheduler_fixes():
    scheduler = (SRC / "HydroScheduler.cpp").read_text()
    datas = (SRC / "HydroDatas.cpp").read_text()
    balancers = (SRC / "HydroBalancers.cpp").read_text()
    rails = (SRC / "HydroRails.cpp").read_text()
    actuators = (SRC / "HydroActuators.cpp").read_text()

    require("Hydro_ReservoirType_FreshWater);" in scheduler[scheduler.index("auto dilutionPumps"):scheduler.index("auto dilutionPumps") + 350],
            "TDS dilution path is not sourcing fresh water")
    require("auto co2Balancer = feedRes->getAirCO2Balancer();" in scheduler, "CO2 setup is fetching the wrong balancer")
    require("airCO2Balancer->setDecrementActuators(decActuators);" in scheduler, "CO2 exhaust is not configured as a decrement path")
    require("isAdditiveData()" in datas[datas.index("HydroCustomAdditiveData::HydroCustomAdditiveData()"):],
            "Custom additive data is validating against the wrong data type")
    require("co2LevelsVar" in datas, "Crop CO2 levels are not deserialized")
    require("_dosingActIndex++;" in balancers, "Timed dosing actuator index does not advance")
    require("HYDRO_CROPS_GROWWEEKS_MAX - 1" in scheduler, "Weekly dosing indexes are not bounded to scheduler storage")
    require("getOutputReservoir()" not in actuators, "Pump actuator still references nonexistent getOutputReservoir()")
    require("_limitTrigger.isTriggered(poll)" in rails, "Regulated rail capacity check does not poll its limit trigger")
    power_units_start = rails.index("void HydroRegulatedRail::setPowerUnits")
    power_units_end = rails.index("HydroSensorAttachment &HydroRegulatedRail::getPowerUsageSensorAttachment", power_units_start)
    require("bumpRevisionIfNeeded();" in rails[power_units_start:power_units_end], "Regulated rail unit changes do not bump revision")


def validate_binary_persistence():
    data = (SRC / "HydroData.cpp").read_text()
    controller = (SRC / "Hydruino.cpp").read_text()
    sensors = (SRC / "HydroSensors.cpp").read_text()
    crops = (SRC / "HydroCrops.cpp").read_text()
    require("const size_t serializedSize = baseDecode._size;" in data and "hydroBinaryDataReadPlan" in data,
            "Binary loader is not respecting serialized record size")
    require("migrateFromBinaryVersion(baseDecode._version)" in data,
            "Binary loader is not applying data-version migrations")
    require("HydroBinarySensorData::migrateFromBinaryVersion" in sensors and "_version = 2;" in sensors,
            "Binary sensor data does not migrate its stable-time field")
    require("HydroTimedCropData::migrateFromBinaryVersion" in crops and "feedIntervalMins =" in crops,
            "Timed crop data does not migrate legacy feeding cadence")
    require("HYDRO_SOFT_ASSERT(!bytesWritten" not in controller,
            "Binary save still reports successful writes as assertion failures")


def validate_readme():
    readme = (ROOT / "README.md").read_text()
    require("UNDER ACTIVE DEVELOPMENT -- WORK IN PROGRESS" not in readme, "README still has WIP banner")


if __name__ == "__main__":
    validate_crop_database()
    validate_factories()
    validate_scheduler_fixes()
    validate_binary_persistence()
    validate_readme()
    print("Hydruino source validation passed")
