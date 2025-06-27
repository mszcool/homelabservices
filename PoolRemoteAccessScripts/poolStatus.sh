#!/bin/bash

# MQTT credentials are defined outside of this script.
host="192.168.99.218"

# Function to subscribe and extract the required fields
subscribe_and_extract() {
  topic="$1"
  mosquitto_sub -t "$topic" -h "$host" -u "$mqttUser" -P "$mqttPwd" -C 1 -F "%j" --pretty | jq -r '
    {
      topic: .topic,
      POWER1: (try (.payload | fromjson | .POWER1) // "N/A"),
      POWER2: (try (.payload | fromjson | .POWER2) // "N/A")
    }
  '
}

# Start subscriptions in background and collect output
subscribe_and_extract "pumplight/stat/RESULT" &
subscribe_and_extract "redoxph/stat/RESULT" &

# Wait a bit to ensure subscriptions are ready
sleep 0.5

# Publish state request
mosquitto_pub -t pumplight/cmnd/State -h "$host" -u "$mqttUser" -P "$mqttPwd" -n
mosquitto_pub -t redoxph/cmnd/State -h "$host" -u "$mqttUser" -P "$mqttPwd" -n

# Wait for background jobs to finish
wait
