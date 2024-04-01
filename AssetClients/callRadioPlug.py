import os
import time
import hmac
import hashlib
import binascii
import requests
import argparse
import secrets
from urllib.parse import urlunparse, urlencode, quote

import radioPlugEntities

#
# Simple Logging Helper
#
def logIfTurnedOn(logMessage):
    logValue = os.environ.get('LOGGING')
    if logValue == 'ON':
        print(logMessage)
#
# This function creates a HMAC signature for the given token and timestamp.
#
def create_hmac_signature(secret_key, token, token_timestamp_str):
    # Update HMAC with token and timestamp
    message = token + token_timestamp_str
    signature = hmac.new(secret_key.encode(), msg=message.encode(), digestmod=hashlib.sha256).digest()
    
    # Convert to hexadecimal for comparison
    signature_hex = binascii.hexlify(signature).decode()

    return signature_hex, message

#
# Creates the final request URL and calls the endpoint.
#
def call_endpoint(switch_ip, headers, operation, queryStr, max_retries=15, retry_interval=5, verb='GET'):
    netloc = switch_ip
    logIfTurnedOn("[Call Endpoint] Netloc: {}".format(netloc))
    logIfTurnedOn("[Call Endpoint] Operation: {}".format(operation))
    logIfTurnedOn("[Call Endpoint] Query string: {}".format(queryStr))
    path = '/{}?{}'.format(operation, queryStr)
    finalUrl = urlunparse(('http', netloc, path, '', '', ''))

    # Retry since the sensor sometimes disconnects from the WiFi due to signal strength issues.
    for retry in range(max_retries):
        try:
            if verb == 'GET':
                response = requests.get(finalUrl, headers=headers)
            elif verb == 'POST':
                response = requests.post(finalUrl, headers=headers)
            elif verb == 'PUT':
                response = requests.put(finalUrl, headers=headers)
            elif verb == 'DELETE':
                response = requests.delete(finalUrl, headers=headers)
            return response
        except requests.exceptions.RequestException as e:
            logIfTurnedOn(f"Request failed: {e}")
            if retry < max_retries - 1:
                logIfTurnedOn(f"Retrying in {retry_interval} seconds...")
                time.sleep(retry_interval)
            else:
                logIfTurnedOn("Max retries reached. Giving up.")
                raise

    return response

#
# Calls the endpoint for getting metadata from the switch.
#
def get_metadata_from_switch(switch_ip, headers):
    logIfTurnedOn("[Metadata] Getting metadata from the switch...")
    response = call_endpoint(switch_ip, headers, 'info', {})
    logIfTurnedOn("[Metadata] Response status code: {}".format(response.status_code))
    logIfTurnedOn("[Metadata] Response body:")
    print(response.text)

    if response.status_code == 200:
        lines = response.text.split('\n')
        info_dict = {}
        for line in lines:
            parts = line.split('=')
            if len(parts) == 2:
                key = parts[0]
                value = parts[1]
                info_dict[key] = value
        status = info_dict['status'] if 'status' in info_dict else None
        sensor_name = info_dict['sensorName'] if 'sensorName' in info_dict else None
        sensor_location = info_dict['sensorLocation'] if 'sensorLocation' in info_dict else None

        return True, status, sensor_name, sensor_location
    else:
        return False, None, None, None

#
# Registers a new switch.
#
def update_metadata_of_switch(switch_ip, headers, sensor_name, sensor_location):
    logIfTurnedOn("[Metadata Update] Setting sensor name and location...")
    response = call_endpoint(switch_ip, headers, 'updateinfo', 'name={}&location={}'.format(sensor_name, sensor_location), verb='PUT')
    logIfTurnedOn("[Metadata Update] Response status code: {}".format(response.status_code))
    logIfTurnedOn("[Metadata Update] Response body:")
    print(response.text)
    if response.status_code == 200:
        return True
    else:
        return False

#
# Registers a new switch with the switch-sensor.
#
def update_switch_data(switch_ip, headers, switch_name, on_command, off_command, is_tri_state, protocol, pulseLength, repeatTransmit):
    logIfTurnedOn("[Save Switch] Storing switch data...")
    response = call_endpoint(
        switch_ip, 
        headers, 
        'updateswitchdata', 
        'name={}&oncommand={}&offcommand={}&istristate={}&protocol={}&pulselength={}&repeattransmit={}'.format(
            switch_name, 
            on_command, 
            off_command, 
            is_tri_state, 
            protocol,
            pulseLength,
            repeatTransmit
        ),
        verb='PUT'
    )
    logIfTurnedOn("[Save Switch] Response status code: {}".format(response.status_code))
    logIfTurnedOn("[Save Switch] Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False

#
# Turns the switch on or off.
#
def turn_switch_on_or_off(switch_ip, headers, switch_name, turn_on):
    logIfTurnedOn("[Switch On/Off] Turning switch {}...".format('on' if turn_on else 'off'))
    response = call_endpoint(
        switch_ip,
        headers,
        'switchon' if turn_on else 'switchoff',
        'name={}'.format(switch_name),
        verb='PUT'
    )

    logIfTurnedOn("[Switch On/Off]  Response status code: {}".format(response.status_code))
    logIfTurnedOn("[Switch On/Off]  Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False

#
# Apply a radio plug configuration file to the target plug
#
def apply_configuration(switch_ip, headers, config_file_name):
    logIfTurnedOn("[Apply config] Applying configuration from file {}...".format(config_file_name))
    # First, check if the file exists
    try:
        with open(config_file_name, 'r') as f:
            json_str = f.read()
            try:
                logIfTurnedOn("[Apply config] Loading configuration data...")
                config = radioPlugEntities.RadioPlugCollection.from_json(json_str)
                logIfTurnedOn("[Apply config] Data loaded, now applying to switch...")
                # Start with updating the metadata / info
                result = update_metadata_of_switch(switch_ip, headers, config.name, config.location)
                if not result:
                    logIfTurnedOn("[Apply config] Failed updating metadata, stopping.")
                    return False
                # Now, register or update each switch part of the config
                for sw in config.plugs:
                    result = update_switch_data(switch_ip, headers, sw.name, sw.onCommand, sw.offCommand, sw.isTriState, sw.protocol, sw.pulseLength, sw.repeatTransmit)
                    if not result:
                        logIfTurnedOn("[Apply config] Failed updating switch {}, stopping.".format(sw.name))
                        return False
                logIfTurnedOn("[Apply config] Done.")
                return True
            except Exception as e:
                logIfTurnedOn("[Apply config] Failed to parse JSON or process JSON contents: {}".format(e))
                return False
    except FileNotFoundError:
        logIfTurnedOn("[Apply config] File not found. Exiting...")
        return False
    
#
# Turn switches on or off by configuration
#
def turn_switches_on_or_off_by_config(switch_ip, headers, config_file_name, turn_on):
    logIfTurnedOn("[Turn switches on/off] Turning switches {}...".format('on' if turn_on else 'off'))
    # First, check if the file exists
    try:
        with open(config_file_name, 'r') as f:
            json_str = f.read()
            try:
                logIfTurnedOn("[Turn switches on/off] Loading configuration data...")
                config = radioPlugEntities.RadioPlugCollection.from_json(json_str)
                logIfTurnedOn("[Turn switches on/off] Data loaded, now turning switches {}...".format('on' if turn_on else 'off'))
                # Now, turn on or off each switch part of the config
                for sw in config.plugs:
                    result = turn_switch_on_or_off(switch_ip, headers, sw.name, turn_on)
                    if not result:
                        logIfTurnedOn("[Turn switches on/off] Failed turning switch {}, stopping.".format(sw.name))
                        return False
                    sleep_time = 0.5
                    #logIfTurnedOn("[Turn switches on/off] Sleeping for {} seconds...".format(sleep_time))
                    time.sleep(sleep_time)
                logIfTurnedOn("[Turn switches on/off] Done.")
                return True
            except Exception as e:
                logIfTurnedOn("[Turn switches on/off] Failed to parse JSON or process JSON contents: {}".format(e))
                return False
    except FileNotFoundError:
        logIfTurnedOn("[Turn switches on/off] File not found. Exiting...")
        return False


#
# Main program execution
#

def main():
    # Create the top-level parser
    parser = argparse.ArgumentParser()
    parser.add_argument('--secret', required=True, help='secret for operations')
    parser.add_argument('--ip', required=True, help='ip address of the switch to work with')
    subparsers = parser.add_subparsers(dest="operation")

    # Create the help command
    parser_info = subparsers.add_parser('help')

    # Create the parser for the "info" command
    parser_info = subparsers.add_parser('info')

    # Create the parser for the "updateinfo" command
    parser_updateinfo = subparsers.add_parser('updateinfo')
    parser_updateinfo.add_argument('--name', required=True)
    parser_updateinfo.add_argument('--location', required=True)

    # Create the parser for the "registerswitch" command
    parser_registerswitch = subparsers.add_parser('registerswitch')
    parser_registerswitch.add_argument('--name', required=True)
    parser_registerswitch.add_argument('--oncommand', required=True)
    parser_registerswitch.add_argument('--offcommand', required=True)
    parser_registerswitch.add_argument('--protocol', required=True, choices=[1, 2, 3, 4, 5], type=int)
    parser_registerswitch.add_argument('--istristate', required=True)
    parser_registerswitch.add_argument('--pulselength', required=True, type=int)
    parser_registerswitch.add_argument('--repeattransmit', required=True, type=int)

    # Create the parser for the "switch" command
    parser_switch = subparsers.add_parser('switch')
    parser_switch.add_argument('--name', required=True)
    parser_switch.add_argument('--status', required=True, choices=['on', 'off'])

    # Apply configuration from configuration JSON file to target
    parser_applyconfig = subparsers.add_parser('applyconfig')
    parser_applyconfig.add_argument('--file', required=True)

    # Apply configuration from configuration JSON file to target
    parser_applyconfig = subparsers.add_parser('turnbyconfig')
    parser_applyconfig.add_argument('--file', required=True)
    parser_applyconfig.add_argument('--status', required=True, choices=['on', 'off'])

    # Parse the arguments
    args = parser.parse_args()

    # Create the token and signature since it will be required for all operations
    token_timestamp_str = str(int(time.time()))
    secret_key = args.secret
    token_data = secrets.token_hex(32)
    signature, token = create_hmac_signature(secret_key, token_data, token_timestamp_str)

    # Prepare the headers, used for every request.
    headers = {
        'Authorization': token_timestamp_str + "|" + token_data + "|" + signature
    }

    logIfTurnedOn("")
    logIfTurnedOn("**** Authentication Details ****")
    logIfTurnedOn("Signature: {}".format(signature))
    logIfTurnedOn("Token: {}".format(token))
    logIfTurnedOn("Timestamp:".format(token_timestamp_str))
    logIfTurnedOn("********************************")
    logIfTurnedOn("")

    # Now, you can access the arguments like this:
    operation = args.operation
    if operation == 'info':
        result, status, sensor_name, sensor_location = get_metadata_from_switch(args.ip, headers)
        if not result:
            logIfTurnedOn("Failed to get metadata. Exiting...")
            SystemExit(1)
    elif operation == 'updateinfo':
        result = update_metadata_of_switch(args.ip, headers, args.name, args.location)
        if not result:
            logIfTurnedOn("Failed to update metadata. Exiting...")
            SystemExit(1)
    elif operation == 'registerswitch':
        result = update_switch_data(args.ip, headers, args.name, args.oncommand, args.offcommand, args.istristate, args.protocol, args.pulselength, args.repeattransmit)
        if not result:
            logIfTurnedOn("Failed to register switch. Exiting...")
            SystemExit(1)
    elif operation == 'switch':
        result = turn_switch_on_or_off(args.ip, headers, args.name, args.status == 'on')
        if not result:
            logIfTurnedOn("Failed to turn switch on/off. Exiting...")
            SystemExit(1)
    elif operation == 'applyconfig':
        result = apply_configuration(args.ip, headers, args.file)
        if not result:
            logIfTurnedOn("Failed to apply configuration. Exiting...")
            SystemExit(1)
    elif operation == 'turnbyconfig':
        result = turn_switches_on_or_off_by_config(args.ip, headers, args.file, args.status == 'on')
        if not result:
            logIfTurnedOn("Failed to turn switches on/off. Exiting...")
            SystemExit(1)
    elif operation == 'help' or operation == None:
        logIfTurnedOn(f"Valid operations are: info, updateinfo, registerswitch, switch")
        logIfTurnedOn(f"info --secret <secretKey> --ip <switchip>: Get the metadata from the switch.")
        logIfTurnedOn(f"updateinfo --secret <secretKey> --ip <switchip> --name <name> --location <location>: Update the metadata of the switch.")
        logIfTurnedOn(f"registerswitch --secret <secretKey> --ip <switchip> --name <name> --oncommand <oncommand> --offcommand <offcommand> --protocol <protocol> --istristate <istristate> --pulselength <length> --repeattransmit <repat-attempts>: Register a new switch with the switch-sensor.")
        logIfTurnedOn(f"switch --secret <secretKey> --ip <switchip> --name <name> --status <status>: Turn the switch on or off.")
        logIfTurnedOn(f"applyconfig --file <pathandfilename>: Apply a radio plug configuration file to the target plug")
        logIfTurnedOn(f"turnbyconfig --file <pathandfilename> --status <status>: Turn switches on or off by configuration")
    else:
        logIfTurnedOn(f"Invalid operation: {operation}")
        logIfTurnedOn(f"Valid operations are: info, updateinfo, registerswitch, switch")
        logIfTurnedOn(f"info: Get the metadata from the switch.")

if __name__ == "__main__":
    main()