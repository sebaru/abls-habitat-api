#/bin/sh

echo Installing mosquitto
dnf -y install mosquitto pwgen

echo Copying mosquitto.conf
cp mosquitto.conf /etc/mosquitto/
chown mosquitto /etc/mosquitto/mosquitto.conf

echo Init DynSec plugins
newpass=`pwgen 64 1`
mosquitto_ctrl dynsec init /etc/mosquitto/dynamic-security.json abls-api $newpass

echo Setting permissions
chown mosquitto:mosquitto /etc/mosquitto/dynamic-security.json

echo Enable Mosquitto
systemctl enable --now mosquitto

echo Set MQTT admin-password in /etc/abls-habitat-api.conf
sed -i "s/MQTT_PASSWORD/$newpass/" /etc/abls-habitat-api.conf
