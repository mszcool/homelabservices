import time
import argparse
import secrets
from urllib.parse import urlunparse, urlencode, quote

import assetRadioPlugEntities
import assetClientUtil as mszutl

#
# Registers a new switch with the switch-sensor.
#
def update_switch_data(switch_ip, headers, switch_name, on_command, off_command, is_tri_state, protocol, pulseLength, repeatTransmit):
    mszutl.logIfTurnedOn("[Save Switch] Storing switch data...")
    response = mszutl.call_endpoint(
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
    mszutl.logIfTurnedOn("[Save Switch] Response status code: {}".format(response.status_code))
    mszutl.logIfTurnedOn("[Save Switch] Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False

#
# Turns the switch on or off.
#
def turn_switch_on_or_off(switch_ip, headers, switch_name, turn_on):
    mszutl.logIfTurnedOn("[Switch On/Off] Turning switch {}...".format('on' if turn_on else 'off'))
    response = mszutl.call_endpoint(
        switch_ip,
        headers,
        'switchon' if turn_on else 'switchoff',
        'name={}'.format(switch_name),
        verb='PUT'
    )

    mszutl.logIfTurnedOn("[Switch On/Off]  Response status code: {}".format(response.status_code))
    mszutl.logIfTurnedOn("[Switch On/Off]  Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False

#
# Apply a radio plug configuration file to the target plug
#
def apply_configuration(switch_ip, headers, config_file_name):
    mszutl.logIfTurnedOn("[Apply config] Applying configuration from file {}...".format(config_file_name))
    # First, check if the file exists
    try:
        with open(config_file_name, 'r') as f:
            json_str = f.read()
            try:
                mszutl.logIfTurnedOn("[Apply config] Loading configuration data...")
                config = assetRadioPlugEntities.RadioPlugCollection.from_json(json_str)
                mszutl.logIfTurnedOn("[Apply config] Data loaded, now applying to switch...")
                # Start with updating the metadata / info
                result = mszutl.update_metadata_of_switch(switch_ip, headers, config.name, config.location)
                if not result:
                    mszutl.logIfTurnedOn("[Apply config] Failed updating metadata, stopping.")
                    return False
                # Now, register or update each switch part of the config
                for sw in config.plugs:
                    result = update_switch_data(switch_ip, headers, sw.name, sw.onCommand, sw.offCommand, sw.isTriState, sw.protocol, sw.pulseLength, sw.repeatTransmit)
                    if not result:
                        mszutl.logIfTurnedOn("[Apply config] Failed updating switch {}, stopping.".format(sw.name))
                        return False
                mszutl.logIfTurnedOn("[Apply config] Done.")
                return True
            except Exception as e:
                mszutl.logIfTurnedOn("[Apply config] Failed to parse JSON or process JSON contents: {}".format(e))
                return False
    except FileNotFoundError:
        mszutl.logIfTurnedOn("[Apply config] File not found. Exiting...")
        return False
    
#
# Turn switches on or off by configuration
#
def turn_switches_on_or_off_by_config(switch_ip, headers, config_file_name, turn_on):
    mszutl.logIfTurnedOn("[Turn switches on/off] Turning switches {}...".format('on' if turn_on else 'off'))
    # First, check if the file exists
    try:
        with open(config_file_name, 'r') as f:
            json_str = f.read()
            try:
                mszutl.logIfTurnedOn("[Turn switches on/off] Loading configuration data...")
                config = assetRadioPlugEntities.RadioPlugCollection.from_json(json_str)
                mszutl.logIfTurnedOn("[Turn switches on/off] Data loaded, now turning switches {}...".format('on' if turn_on else 'off'))
                # Now, turn on or off each switch part of the config
                for sw in config.plugs:
                    result = turn_switch_on_or_off(switch_ip, headers, sw.name, turn_on)
                    if not result:
                        mszutl.logIfTurnedOn("[Turn switches on/off] Failed turning switch {}, stopping.".format(sw.name))
                        return False
                    sleep_time = 0.5
                    #logIfTurnedOn("[Turn switches on/off] Sleeping for {} seconds...".format(sleep_time))
                    time.sleep(sleep_time)
                mszutl.logIfTurnedOn("[Turn switches on/off] Done.")
                return True
            except Exception as e:
                mszutl.logIfTurnedOn("[Turn switches on/off] Failed to parse JSON or process JSON contents: {}".format(e))
                return False
    except FileNotFoundError:
        mszutl.logIfTurnedOn("[Turn switches on/off] File not found. Exiting...")
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
    signature, token = mszutl.create_hmac_signature(secret_key, token_data, token_timestamp_str)

    # Prepare the headers, used for every request.
    headers = {
        'Authorization': token_timestamp_str + "|" + token_data + "|" + signature
    }

    mszutl.logIfTurnedOn("")
    mszutl.logIfTurnedOn("**** Authentication Details ****")
    mszutl.logIfTurnedOn("Signature: {}".format(signature))
    mszutl.logIfTurnedOn("Token: {}".format(token))
    mszutl.logIfTurnedOn("Timestamp:".format(token_timestamp_str))
    mszutl.logIfTurnedOn("********************************")
    mszutl.logIfTurnedOn("")

    # Now, you can access the arguments like this:
    operation = args.operation
    if operation == 'info':
        result, status, sensor_name, sensor_location = mszutl.get_metadata_from_switch(args.ip, headers)
        if not result:
            mszutl.logIfTurnedOn("Failed to get metadata. Exiting...")
            SystemExit(1)
    elif operation == 'updateinfo':
        result = mszutl.update_metadata_of_switch(args.ip, headers, args.name, args.location)
        if not result:
            mszutl.logIfTurnedOn("Failed to update metadata. Exiting...")
            SystemExit(1)
    elif operation == 'registerswitch':
        result = update_switch_data(args.ip, headers, args.name, args.oncommand, args.offcommand, args.istristate, args.protocol, args.pulselength, args.repeattransmit)
        if not result:
            mszutl.logIfTurnedOn("Failed to register switch. Exiting...")
            SystemExit(1)
    elif operation == 'switch':
        result = turn_switch_on_or_off(args.ip, headers, args.name, args.status == 'on')
        if not result:
            mszutl.logIfTurnedOn("Failed to turn switch on/off. Exiting...")
            SystemExit(1)
    elif operation == 'applyconfig':
        result = apply_configuration(args.ip, headers, args.file)
        if not result:
            mszutl.logIfTurnedOn("Failed to apply configuration. Exiting...")
            SystemExit(1)
    elif operation == 'turnbyconfig':
        result = turn_switches_on_or_off_by_config(args.ip, headers, args.file, args.status == 'on')
        if not result:
            mszutl.logIfTurnedOn("Failed to turn switches on/off. Exiting...")
            SystemExit(1)
    elif operation == 'help' or operation == None:
        mszutl.logIfTurnedOn(f"Valid operations are: info, updateinfo, registerswitch, switch")
        mszutl.logIfTurnedOn(f"info --secret <secretKey> --ip <switchip>: Get the metadata from the switch.")
        mszutl.logIfTurnedOn(f"updateinfo --secret <secretKey> --ip <switchip> --name <name> --location <location>: Update the metadata of the switch.")
        mszutl.logIfTurnedOn(f"registerswitch --secret <secretKey> --ip <switchip> --name <name> --oncommand <oncommand> --offcommand <offcommand> --protocol <protocol> --istristate <istristate> --pulselength <length> --repeattransmit <repat-attempts>: Register a new switch with the switch-sensor.")
        mszutl.logIfTurnedOn(f"switch --secret <secretKey> --ip <switchip> --name <name> --status <status>: Turn the switch on or off.")
        mszutl.logIfTurnedOn(f"applyconfig --file <pathandfilename>: Apply a radio plug configuration file to the target plug")
        mszutl.logIfTurnedOn(f"turnbyconfig --file <pathandfilename> --status <status>: Turn switches on or off by configuration")
    else:
        mszutl.logIfTurnedOn(f"Invalid operation: {operation}")
        mszutl.logIfTurnedOn(f"Valid operations are: info, updateinfo, registerswitch, switch")
        mszutl.logIfTurnedOn(f"info: Get the metadata from the switch.")

if __name__ == "__main__":
    main()