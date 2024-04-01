import time
import argparse
import secrets
from urllib.parse import urlunparse, urlencode, quote

import assetDepthSensorEntities as dentities
import assetClientUtil as mszutl

#
# Get the current configuration of the depth sensor
#
def get_depth_sensor_config(sensor_ip, headers):
    mszutl.logIfTurnedOn("[Depth Config] Getting depth sensor configuration...")
    response = mszutl.call_endpoint(sensor_ip, headers, 'config', '', verb='GET')
    # Parse the response
    if response.status_code == 200:
        mszutl.logIfTurnedOn("[Depth Config] Response status code: {}".format(response.status_code))
        mszutl.logIfTurnedOn("[Depth Config] Response body:")
        print(response.text)
        config = dentities.DepthSensorConfig.from_json(response.text)
        return True, config
    else:
        print("Failed to get depth sensor configuration. Response code: {}".format(response.status_code))
        return False, None
#
# Updating the configuration of the depth sensor
#
def update_depth_sensor_config(sensor_ip, headers, config):
    mszutl.logIfTurnedOn("[Depth Config Update] Updating depth sensor configuration...")
    response = mszutl.call_endpoint(
        sensor_ip,
        headers,
        'config',
        'measurementintervalseconds={}&measurementstokeep={}'.format(
            config.measureIntervalInSeconds,
            config.measurementsToKeep
        ),
        verb='PUT'
    )

    mszutl.logIfTurnedOn("[Depth Config Update] Response status code: {}".format(response.status_code))
    mszutl.logIfTurnedOn("[Depth Config Update] Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False
    
#
# Get the available measurements from the sensor
#
def get_depth_sensor_measurements(sensor_ip, headers):
    mszutl.logIfTurnedOn("[Depth Measurements] Getting depth sensor measurements...")
    response = mszutl.call_endpoint(sensor_ip, headers, 'measurements', '', verb='GET')

    mszutl.logIfTurnedOn("[Depth Measurements] Response status code: {}".format(response.status_code))
    mszutl.logIfTurnedOn("[Depth Measurements] Response body:")
    print(response.text)

    # Parse the response
    if response.status_code == 200:
        measurements = dentities.DepthSensorMeasurementCollection.from_json(response.text)
        return True, measurements
    else:
        return False, None
    
#
# Purge the measurements available from the sensor
#
def purge_depth_sensor_measurements(sensor_ip, headers):
    mszutl.logIfTurnedOn("[Depth Measurements Purge] Purging depth sensor measurements...")
    response = mszutl.call_endpoint(sensor_ip, headers, 'measurements', '', verb='DELETE')

    mszutl.logIfTurnedOn("[Depth Measurements Purge] Response status code: {}".format(response.status_code))
    mszutl.logIfTurnedOn("[Depth Measurements Purge] Response body:")
    print(response.text)

    if response.status_code == 200:
        return True
    else:
        return False
    
#
# Main function
#
    
def main():
    print("Asset Depth Sensor Management Utility")
    # Create the top-level parser
    parser = argparse.ArgumentParser(description='Manage the depth sensor')
    parser.add_argument('--secret', required=False, help='The secret key used for the authentication')
    parser.add_argument('--ip', required=False, help='The IP address of the depth sensor')
    subparsers = parser.add_subparsers(dest="operation", help='The operation to perform')

    # Create the help command
    help_parser = subparsers.add_parser('help', help='Show the help message')

    # Create the parser for the info command
    info_parser = subparsers.add_parser('info', help='Get the current configuration of the depth sensor')

    # Create the parser for the update info command
    update_info_parser = subparsers.add_parser('updateinfo', help='Update the configuration of the depth sensor')
    update_info_parser.add_argument('--name', required=True, help='The friendly name of the sensor to set.')
    update_info_parser.add_argument('--location', required=True, help='The location of the sensor to set.')

    # Create the parser for the depth sensor configuration
    get_config_parser = subparsers.add_parser('config', help='Get or update the configuration of the depth sensor')

    # Create the parser for updating the depth sensor configuration
    update_config_parser = subparsers.add_parser('updateconfig', help='Update the configuration of the depth sensor')
    update_config_parser.add_argument('--interval', type=int, help='The interval in seconds between measurements')
    update_config_parser.add_argument('--keep', type=int, help='The number of measurements to keep')

    # Create the parser for the depth sensor measurements
    get_measurements_parser = subparsers.add_parser('measurements', help='Get the measurements from the depth sensor')

    # Create the parser for purging the depth sensor measurements
    purge_measurements_parser = subparsers.add_parser('purge', help='Purge the measurements from the depth sensor')

    # Parse the arguments
    args = parser.parse_args()

    if args.operation != 'help' and (args.secret is None or args.ip is None):
        print("The --secret and --ip arguments are required for all operations.")
        SystemExit(1)
    elif args.operation == 'help':
        args.secret = ""

    # Create the token and the signature since it will be required for all operations
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

    # Now let's perform the operation
    operation = args.operation
    if operation == 'info':
        result = mszutl.get_metadata_from_switch(args.ip, headers)
        if not result:
            print("Failed to get the depth sensor metadata.")
            SystemExit(1)
    elif operation == 'updateinfo':
        result = mszutl.update_metadata_of_switch(args.ip, headers, args.name, args.location)
        if not result:
            print("Failed to update the depth sensor metadata.")
            SystemExit(1)
    elif operation == 'config':
        result = get_depth_sensor_config(args.ip, headers)
        if not result:
            print("Failed to get the depth sensor configuration.")
            SystemExit(1)
    elif operation == 'updateconfig':
        config = dentities.DepthSensorConfig(False, args.interval, args.keep)
        result = update_depth_sensor_config(args.ip, headers, config)
        if not result:
            print("Failed to update the depth sensor configuration.")
            SystemExit(1)
    elif operation == 'measurements':
        result, measurements = get_depth_sensor_measurements(args.ip, headers)
        if not result:
            print("Failed to get the depth sensor measurements.")
            SystemExit(1)
    elif operation == 'purge':
        result = purge_depth_sensor_measurements(args.ip, headers)
        if not result:
            print("Failed to purge the depth sensor measurements.")
            SystemExit(1)
    elif operation == 'help':
        parser.print_help()
        parser.print_usage()
    else:
        print("Invalid operation: {}".format(operation))
        print("Use the 'help' command to see the available operations.")
        print("Valid operations are: info, updateinfo, config, updateconfig, help")
        SystemExit(1)

if __name__ == '__main__':
    main()