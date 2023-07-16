import time
import hmac
import hashlib
import binascii
import requests

def create_hmac_signature(secret_key, token, token_timestamp_str):
    # Update HMAC with token and timestamp
    message = token + token_timestamp_str
    signature = hmac.new(secret_key.encode(), msg=message.encode(), digestmod=hashlib.sha256).digest()
    
    # Convert to hexadecimal for comparison
    signature_hex = binascii.hexlify(signature).decode()

    return signature_hex, message

token_timestamp_str = str(int(time.time()))
secret_key = 'yourSecret123!here'
token_data = 'thisIsMyTestTokenForTheCode__!!'

signature, token = create_hmac_signature(secret_key, token_data, token_timestamp_str)

print("Signature:", signature)
print("Token:", token)
print("Timestamp:", token_timestamp_str)

# Prepare the headers, used for every request.
headers = {
    'Authorization': token_timestamp_str + "|" + token_data + "|" + signature
}

# First, get the metadata from the switch
print("Getting metadata from the switch...")
url = 'http://192.168.99.42/info'
response = requests.get(url, headers=headers)
print("Response status code:", response.status_code)
print("Response body:", response.text)
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

if sensor_name == None or sensor_location == None:
    print("Setting sensor name and location...")
    url = 'http://192.168.99.42/updatemetadata?name=testSensor&location=testLocation'
    response = requests.get(url, headers=headers)
    print("Response status code:", response.status_code)
    print("Response body:", response.text)

# Now store the switch data
print("Storing switch data...")
url = 'http://192.168.99.42/updateswitchdata?name=testsw1&oncommand=0101010&offcommand=010101&isTriState=false&protocol=5'
response = requests.get(url, headers=headers)
print("Response status code:", response.status_code)
print("Response body:", response.text)

# Assuming 'url' contains the URL of the server endpoint
url = 'http://192.168.99.42/switchon?name=testsw1'
response = requests.get(url, headers=headers)

print("Response status code:", response.status_code)
print("Response body:", response.text)

# Wait for the server to process the request
delay = 20
print("Waiting for", delay, "seconds...")
time.sleep(delay)

# Now, turn the switch off, again
url = 'http://192.168.99.42/switchoff?name=testsw1'
response = requests.get(url, headers=headers)

print("Response status code:", response.status_code)
print("Response body:", response.text)