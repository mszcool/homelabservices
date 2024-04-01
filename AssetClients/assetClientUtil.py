import os
import time
import hmac
import hashlib
import binascii
import requests
import argparse
import secrets
from urllib.parse import urlunparse, urlencode, quote

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