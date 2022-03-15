#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
   echo "Only user root can run this script (or sudo)."
   exit 1
fi

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf install -y git libtool automake autoconf gcc gcc-c++
  dnf install -y glib2-devel openssl libsoup-devel json-glib-devel
  dnf install -y mariadb-devel libuuid-devel

fi


if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ]
 then
  echo "Installing debian dependencies"

  if [ "$SOCLE" = "raspbian" ]
   then
    apt install -y gcc-8-base
  fi

  apt install -y git libtool automake autoconf gcc git openssl
  apt install -y libglib2.0-dev libssl-dev default-libmysqlclient-dev
  apt install -y libmariadbclient-dev libjson-glib-dev libsoup2.4-dev
fi

git clone https://github.com/sebaru/Abls-Habitat-API.git ablsapi
cd ablsapi
echo "Compiling and installing"
./autogen.sh
make install
cd ..
rm -rf ablsapi
systemctl daemon-reload

echo "Please edit /etc/abls-habitat-api.conf before starting"
