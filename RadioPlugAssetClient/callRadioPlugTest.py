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

# Assuming 'url' contains the URL of the server endpoint
url = 'http://192.168.99.42/switchon'

# Send a request with the token and the signature in the headers
headers = {
    'Authorization': token_timestamp_str + "|" + token_data + "|" + signature
}
response = requests.get(url, headers=headers)

print("Response status code:", response.status_code)
print("Response body:", response.text)

# Wait for the server to process the request
delay = 20
print("Waiting for", delay, "seconds...")
time.sleep(delay)

# Now, turn the switch off, again
url = 'http://192.168.99.42/switchoff'
response = requests.get(url, headers=headers)

print("Response status code:", response.status_code)
print("Response body:", response.text)