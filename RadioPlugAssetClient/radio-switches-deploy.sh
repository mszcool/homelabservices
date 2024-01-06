#!/bin/bash

# Python Script Path
pythonScriptPath="./"

# Get the first parameter, which is the secret key
secretKey=$1
switchIp=$2
switchFile=$3

# Set the metadata for the switch
python3 $pythonScriptPath/callRadioPlug.py --secret "$secretKey" \
                                           --ip "$switchIp" \
                                           applyconfig \
                                           --applyconfig "$switchFile"