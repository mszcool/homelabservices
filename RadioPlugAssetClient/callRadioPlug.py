import time
import hmac
import hashlib
import binascii
import requests
import argparse
import secrets
from urllib.parse import urlunparse, urlencode, quote

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
def call_endpoint(switch_ip, headers, operation, queryStr):
    netloc = switch_ip
    print("[Call Endpoint] Netloc:", netloc)
    print("[Call Endpoint] Operation:", operation)
    print("[Call Endpoint] Query string:", queryStr)
    path = '/{}?{}'.format(operation, queryStr)
    finalUrl = urlunparse(('http', netloc, path, '', '', ''))
    response = requests.get(finalUrl, headers=headers)
    return response

#
# Calls the endpoint for getting metadata from the switch.
#
def get_metadata_from_switch(switch_ip, headers):
    print("[Metadata] Getting metadata from the switch...")
    response = call_endpoint(switch_ip, headers, 'info', {})
    print("[Metadata] Response status code:", response.status_code)
    print("[Metadata] Response body:", response.text)

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
    print("[Metadata Update] Setting sensor name and location...")
    response = call_endpoint(switch_ip, headers, 'updatemetadata', 'name={}&location={}'.format(sensor_name, sensor_location))
    print("[Metadata Update] Response status code:", response.status_code)
    print("[Metadata Update] Response body:", response.text)
    if response.status_code == 200:
        return True
    else:
        return False

#
# Registers a new switch with the switch-sensor.
#
def update_switch_data(switch_ip, headers, switch_name, on_command, off_command, is_tri_state, protocol):
    print("[Save Switch] Storing switch data...")
    response = call_endpoint(
        switch_ip, 
        headers, 
        'updateswitchdata', 
        'name={}&oncommand={}&offcommand={}&istristate={}&protocol={}'.format(
            switch_name, 
            on_command, 
            off_command, 
            is_tri_state, 
            protocol
        )
    )
    print("[Save Switch] Response status code:", response.status_code)
    print("[Save Switch] Response body:", response.text)

    if response.status_code == 200:
        return True
    else:
        return False

#
# Turns the switch on or off.
#
def turn_switch_on_or_off(switch_ip, headers, switch_name, turn_on):
    print("[Switch On/Off] Turning switch {}...".format('on' if turn_on else 'off'))
    response = call_endpoint(
        switch_ip,
        headers,
        'switchon' if turn_on else 'switchoff',
        'name={}'.format(switch_name)
    )

    print("[Switch On/Off]  Response status code:", response.status_code)
    print("[Switch On/Off]  Response body:", response.text)

    if response.status_code == 200:
        return True
    else:
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

    # Create the parser for the "switch" command
    parser_switch = subparsers.add_parser('switch')
    parser_switch.add_argument('--name', required=True)
    parser_switch.add_argument('--status', required=True, choices=['on', 'off'])

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

    print("")
    print("**** Authentication Details ****")
    print("Signature:", signature)
    print("Token:", token)
    print("Timestamp:", token_timestamp_str)
    print("********************************")
    print("")

    # Now, you can access the arguments like this:
    operation = args.operation
    if operation == 'info':
        status, sensor_name, sensor_location = get_metadata_from_switch(args.ip, headers)
    elif operation == 'updateinfo':
        result = update_metadata_of_switch(args.ip, headers, args.name, args.location)
        if not result:
            print("Failed to update metadata. Exiting...")
            SystemExit(1)
    elif operation == 'registerswitch':
        result = update_switch_data(args.ip, headers, args.name, args.oncommand, args.offcommand, args.istristate, args.protocol)
        if not result:
            print("Failed to register switch. Exiting...")
            SystemExit(1)
    elif operation == 'switch':
        result = turn_switch_on_or_off(args.ip, headers, args.name, args.status == 'on')
        if not result:
            print("Failed to turn switch on/off. Exiting...")
            SystemExit(1)
    elif operation == 'help' or operation == None:
        print(f"Valid operations are: info, updateinfo, registerswitch, switch")
        print(f"info --secret <secretKey> --ip <switchip>: Get the metadata from the switch.")
        print(f"updateinfo --secret <secretKey> --ip <switchip> --name <name> --location <location>: Update the metadata of the switch.")
        print(f"registerswitch --secret <secretKey> --ip <switchip> --name <name> --oncommand <oncommand> --offcommand <offcommand> --protocol <protocol> --istristate <istristate>: Register a new switch with the switch-sensor.")
        print(f"switch --secret <secretKey> --ip <switchip> --name <name> --status <status>: Turn the switch on or off.")
    else:
        print(f"Invalid operation: {operation}")
        print(f"Valid operations are: info, updateinfo, registerswitch, switch")
        print(f"info: Get the metadata from the switch.")

if __name__ == "__main__":
    main()