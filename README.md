# 2025 Lunabotics Creed system Setup

# Motor Control & CAN Adapter System

This project controls a motor using a USB-to-CAN adapter on a Jetson board with ROS 2 Foxy, CTRE Phoenix6, and SLcand. It automatically brings up the CAN interface, runs the motor control code on boot, and waits for remote joystick commands from a laptop running ROS 2 Humble.

## Overview

- **Jetson Board (Motor Controller):**  
  - Runs ROS 2 Foxy.
  - Uses a persistent udev rule to ensure the USB-to-CAN adapter is always accessible as `/dev/can0` (regardless of whether it appears as ttyACM0 or ttyACM1).
  - A startup script brings up the CAN interface and launches the motor control executable automatically on boot.
  
- **Laptop (Joystick Controller):**  
  - Runs ROS 2 Humble.
  - Connects a Logitech controller and uses the ROS 2 joy node to publish joystick commands.
  - Must be on the same Wi-Fi network as the Jetson with the same `ROS_DOMAIN_ID` (e.g., 42) to enable ROS 2 communication.

## Setup & Operation Steps

1. **Persistent CAN Adapter Device (Jetson):**
   - A udev rule (`/etc/udev/rules.d/99-can.rules`) creates a symlink `/dev/can0` for the USB-to-CAN adapter based on its stable properties (vendor, product, serial, or physical USB path).
   - Example udev rule:
     ```udev
     SUBSYSTEM=="tty", ENV{ID_VENDOR_ID}=="****", ENV{ID_MODEL_ID}=="117e", ENV{ID_SERIAL_SHORT}=="********", SYMLINK+="ttyCAN"
     ```
   - Reload rules with:
     ```bash
     sudo udevadm control --reload-rules && sudo udevadm trigger
     ```

2. **Startup Script (Jetson):**
   - The `start_motor_system.sh` script:
     - Sources the ROS 2 setup.
     - Brings up the CAN interface using SLcand with the persistent device `/dev/ttyCAN`.
     - Waits for the CAN network interface (e.g., `can0`) to be created.
     - Launches the motor control executable.
   - Example snippet from the script:
     ```bash
     #!/bin/bash
     source /opt/ros/foxy/setup.bash

     interface="can0"         # CAN network interface
     device="/dev/can0"       # Persistent device name from udev rule

     sudo pkill slcand
     sudo slcand -o -c -s8 $device $interface

     echo "Waiting for CAN interface $interface..."
     for i in {1..5}; do
         if ip link show $interface >/dev/null 2>&1; then
             echo "CAN interface $interface detected!"
             break
         fi
         sleep 1
     done

     sudo ip link set $interface up type can bitrate 500000
     ```
     Full code can be located in the shell files folder.
     
   - This script is set to run automatically on boot via a systemd service.

3. **Systemd Service (Jetson):**
   - A systemd service (`/etc/systemd/system/motor.service`) is configured to run the startup script on boot.
   - Example service file:
     ```ini
     [Unit]
     Description=Motor Control System
     After=network.target

     [Service]
     Type=simple
     ExecStart=/home/creed/start_motor_system.sh
     Restart=on-failure
     User=creed
     Environment="ROS_DOMAIN_ID=42"

     [Install]
     WantedBy=multi-user.target
     ```
   - Enable and start the service:
     ```bash
     sudo systemctl daemon-reload
     sudo systemctl enable motor.service
     sudo systemctl start motor.service
     ```

4. **ROS 2 Communication Between Jetson and Laptop:**
   - Both devices must be connected to the same Wi-Fi network.
   - Both devices must have the same `ROS_DOMAIN_ID` (e.g., set to 42).
   - This allows the Jetson to receive joystick commands published by the laptop.

5. **Joystick Control (Laptop):**
   - Connect a Logitech controller to the laptop.
   - Launch the joy node:
     ```bash
     ros2 run joy joy_node
     ```
   - Joystick commands will be published to `/joy` and received by the Jetson to control the motor.

## Summary

- **On Boot:**  
  The Jetson’s systemd service runs the startup script. The udev rule ensures the CAN adapter is available as `/dev/can0`, and the script brings up the CAN interface and launches the motor control code automatically.
  
- **Remote Control:**  
  As long as the Jetson and laptop are on the same network and share the same `ROS_DOMAIN_ID`, the Jetson will receive joystick commands from the laptop (via the joy node), enabling remote control of the motor.
  
- **Development & Updates:**  
  To update the motor control code, remake the builde file using the following commands:
  ```bash
  cd build
  cmake ..
  make
  ./(project name) #for testing the project
  ```
  If there are any changes to the code file, or new path file for the build project, you must chnage the file in the following shell file then running the following line to confirm changes:
  ```bash
  sudo nano run_jetson_code.sh
  chmod +x run_jetson_code.sh
  ```
  Since service is already running and you want to apply the update, you must run this next:
  ```bash
  sudo systemctl daemon-reload
  sudo systemctl restart motor.service
  ```

