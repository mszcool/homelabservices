#!/bin/bash

# Python Script Path
pythonScriptPath="./"

# Get the first parameter, which is the secret key
secretKey=$1
switchIp=$2

# Now let's call the corresponding endpoint for turning the switch on or off.
echo "Getting information..."
python3 $pythonScriptPath/callRadioPlug.py --secret "$secretKey" \
                                           --ip "$switchIp" \
                                           info