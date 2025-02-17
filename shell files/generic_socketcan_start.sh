#!/bin/bash

# Setup CAN for RH02 USB-to-CAN Adapter
interface="can0"
device="/dev/can0" 

# Kill any existing slcand processes
sudo pkill slcand

# Start slcand with correct settings
sudo slcand -o -c -s8 $device $interface

# Wait for `can0` to appear
echo "Waiting for CAN interface to be created..."
for i in {1..5}; do
    if ip link show $interface >/dev/null 2>&1; then
        echo "CAN interface $interface detected!"
        break
    fi
    sleep 1
done

# Check if `can0` was successfully created
if ! ip link show $interface >/dev/null 2>&1; then
    echo "Error: CAN interface $interface did not initialize correctly."
    exit 1
fi

# Bring up the interface
sudo ifconfig $interface up

# Set TX queue length
sudo ifconfig $interface txqueuelen 1000

echo "CAN interface $interface is now fully set up!"

