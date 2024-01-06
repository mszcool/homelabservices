import json

class RadioPlug:
    def __init__(self, name, onCommand, offCommand, isTriState, protocol):
        self.name = name
        self.onCommand = onCommand
        self.offCommand = offCommand
        self.isTriState = isTriState
        self.protocol = protocol

class RadioPlugCollection:
    def __init__(self, name, location, plugs):
        self.name = name
        self.location = location
        self.plugs = plugs

    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)
    
    @classmethod
    def from_json(cls, json_str):
        json_dict = json.loads(json_str)
        plugs = [RadioPlug(**plug) for plug in json_dict['plugs']]
        return cls(
            json_dict['name'],
            json_dict['location'],
            plugs
        )