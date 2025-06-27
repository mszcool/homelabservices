# Pump ON
mosquitto_pub -t pumplight/cmnd/POWER1 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.58/cm?user=admin&password=$wifiPassword&cmnd=Power1%20OFF"
# Light ON
mosquitto_pub -t pumplight/cmnd/POWER2 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.58/cm?user=admin&password=$wifiPassword&cmnd=Power2%20OFF"
# Redox ON
mosquitto_pub -t redoxph/cmnd/POWER1 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.61/cm?user=admin&password=$wifiPassword&cmnd=Power1%20OFF"
# PH ON
mosquitto_pub -t redoxph/cmnd/POWER2 -h 192.168.99.218 -u $mqttUser -P $mqttPwd -m "ON"
#curl -s "http://192.168.99.61/cm?user=admin&password=$wifiPassword&cmnd=Power2%20OFF"
