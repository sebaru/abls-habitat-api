#/bin/sh

echo Installing mosquitto
dnf -y install mosquitto pwgen

echo Copying mosquitto.conf
cp mosquitto.conf /etc/mosquitto/

echo Erase old config file
rm /etc/mosquitto/dynamic-security.json

echo Init DynSec plugins
newpass=`pwgen 64 1`
mosquitto_ctrl dynsec init /etc/mosquitto/dynamic-security.json api $newpass

echo Setting permissions
chown mosquitto:mosquitto -R /etc/mosquitto

#chmod a+x /etc/letsencrypt/archive/
#chown mosquitto:mosquitto /etc/letsencrypt/archive/fqdn -R

echo Enable Mosquitto
systemctl enable mosquitto
systemctl restart mosquitto

mosquitto_ctrl -u api -P $newpass dynsec addRoleACL admin publishClientReceive "#" allow
mosquitto_ctrl -u api -P $newpass dynsec addRoleACL admin publishClientSend "#" allow

echo Set MQTT admin-password in /etc/abls-habitat-api.conf
sed -i "s/^.*\"mqtt_password\".*/  \"mqtt_password\": \"$newpass\",/" /etc/abls-habitat-api.conf
