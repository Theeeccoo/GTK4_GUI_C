#!/bin/sh
LIST_OF_APPS = "build-essentials pgk-config lib-gtk4-dev libx11-dev"
apt-get update # Getting the latest packages
apt install $LIST_OF_APPS -y # Installing all necessary packages