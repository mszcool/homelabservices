import json

class RadioPlug:
    def __init__(self, name, onCommand, offCommand, isTriState, protocol, pulseLength, repeatTransmit):
        self.name = name
        self.onCommand = onCommand
        self.offCommand = offCommand
        self.isTriState = isTriState
        self.protocol = protocol
        self.pulseLength = pulseLength
        self.repeatTransmit = repeatTransmit

class RadioReceive:
    def __init__(self, receiveValue, receiveProtocol, receiveTopic, receiveCommand, receiveName):
        self.receiveValue = receiveValue
        self.receiveProtocol = receiveProtocol
        self.receiveTopic = receiveTopic
        self.receiveCommand = receiveCommand
        self.receiveName = receiveName

class RadioPlugCollection:
    def __init__(self, name, location, mqttServer, mqttPort, mqttUser, mqttPassword, plugs, receivers):
        self.name = name
        self.location = location
        self.plugs = plugs
        self.receivers = receivers
        self.mqttServer = mqttServer
        self.mqttPort = mqttPort
        self.mqttUser = mqttUser
        self.mqttPassword = mqttPassword

    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)
    
    @classmethod
    def from_json(cls, json_str):
        json_dict = json.loads(json_str)
        plugs = [RadioPlug(**plug) for plug in json_dict['plugs']]
        receivers = [RadioReceive(**receiver) for receiver in json_dict['receivers']]
        return cls(
            json_dict['name'],
            json_dict['location'],
            json_dict['mqttServer'],
            json_dict['mqttPort'],
            json_dict['mqttUser'],
            json_dict['mqttPassword'],
            plugs,
            receivers
        )