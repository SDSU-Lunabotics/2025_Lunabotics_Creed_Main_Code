#!/bin/bash
# Source ROS 2 setup files
source /opt/ros/foxy/setup.bash

# Run the Python script
# python3 ~/Creed/test.py

/home/creed/generic_socketcan_start.sh

/home/creed/ros2_ws/build/Motor-Test
