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