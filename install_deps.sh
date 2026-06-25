#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
   echo "Only user root can run this script (or sudo)."
   exit 1
fi

groupadd abls

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  sudo dnf install -y git libtool cmake gcc gcc-c++ bison flex
  sudo dnf install -y glib2-devel openssl libsoup3-devel json-glib-devel libjwt-devel
  sudo dnf install -y mariadb-devel libuuid-devel mosquitto-devel libmemcached-awesome-devel

fi

if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ] || [ "$SOCLE" = "ubuntu" ]
 then
  echo "Installing debian/ubuntu dependencies"

  sudo apt update -y

  if [ "$SOCLE" = "raspbian" ]
   then
    sudo apt install -y gcc-8-base
  fi

  if [ "$SOCLE" = "ubuntu" ]
   then
    sudo apt install -y software-properties-common
    sudo add-apt-repository universe -y
    sudo apt update -y
  fi

  sudo apt install -y git libtool cmake gcc g++ bison flex openssl pkg-config
  sudo apt install -y libglib2.0-dev libssl-dev
  sudo apt install -y libmariadb-dev libjson-glib-dev libsoup-3.0-dev libmemcached-dev
  sudo apt install -y libjwt-dev uuid-dev libmosquitto-dev
fi