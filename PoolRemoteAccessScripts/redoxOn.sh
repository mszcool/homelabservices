# Pump on
mosquitto_pub -t pumplight/cmnd/POWER1 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.58/cm?user=admin&password=$wifiPassword&cmnd=Power1%20ON"
# Redox ON
mosquitto_pub -t redoxph/cmnd/POWER1 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.61/cm?user=admin&password=$wifiPassword&cmnd=Power1%20ON"
