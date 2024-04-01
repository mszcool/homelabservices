import json

#
# Used to retrieve the depth sensor configuration
#
class DepthSensorConfig:
    def __init__(self, isDefault, measureIntervalInSeconds, measurementsToKeep):
        self.isDefault = isDefault
        self.measureIntervalInSeconds = measureIntervalInSeconds
        self.measurementsToKeep = measurementsToKeep
    
    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)
    
    @classmethod
    def from_json(cls, json_str):
        json_dict = json.loads(json_str)
        return cls(
            json_dict['isDefault'],
            json_dict['measurementIntervalSeconds'],
            json_dict['measurementsToKeep']
        )

#
# Used to describe a single measurement.
#
class DepthSensorMeasurement:
    def __init__(self, measureTime, centimeters, retrievedBefore):
        self.measureTime = measureTime
        self.centimeters = centimeters
        self.retrievedBefore = retrievedBefore

    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)
    
    @classmethod
    def from_json(cls, json_str):
        json_dict = json.loads(json_str)
        return cls(
            json_dict['measureTime'],
            json_dict['centimeters'],
            json_dict['retrievedBefore']
        )

#
# Used to describe a list of measurements.
#
class DepthSensorMeasurementCollection:
    def __init__(self, measurements):
        self.measurements = measurements

    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)
    
    @classmethod
    def from_json(cls, json_str):
        json_dict = json.loads(json_str)
        measurements = [DepthSensorMeasurement(**measurement) for measurement in json_dict['measurements']]
        return cls(
            measurements
        )