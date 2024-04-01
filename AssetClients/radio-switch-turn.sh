#!/bin/bash

# Get the first parameter, which is the secret key
secretKey=$1
switchIp=$2
switchName=$3
switchOn=$4

# Python Script Path
pythonScriptPath="./"

if [ "$switchOn" != "on" or "$switchOn" != "off" ]; then
    echo "Switch $switchOn is not an allowed option."
    exit 1
fi

# Now let's call the corresponding endpoint for turning the switch on or off.
echo "Turning switch $switchName $swtichOn..."
python3 $pythonScriptPath/callRadioPlug.py --secret "$secretKey" \
                                           --ip "$switchIp" \
                                           switch \
                                           --name "$switchName" \
                                           --status "$switchOn"

if [ $? -eq 0 ]; then
    echo "Switch $switchName turned $switchOn successfully."
    exit 0
else
    echo "Switch $switchName failed to turn $switchOn."
    exit 1
fi