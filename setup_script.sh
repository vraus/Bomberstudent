#!/bin/bash

# Check if the script is being run with superuser rights
if [ "$EUID" -ne 0 ]
  then echo "Please run with superuser rights (sudo)"
  exit
fi

# Update the environment
apt-get update

# Install autoreconf, libtool, and automake
apt-get install autoconf libtool automake

# Navigate to the jansson-master directory
cd jansson-master

# Autoreconf (if necessary)
autoreconf -i

# Configure, make, and install Jansson
./configure
make
make install