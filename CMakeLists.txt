cmake_minimum_required(VERSION 3.6)
project(Creed)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Prefer pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find required packages
find_package(phoenix6 REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(rosidl_typesupport_fastrtps_cpp REQUIRED)

# Configure units library (using iostream)
add_definitions(-DUNIT_LIB_DISABLE_FMT -DUNIT_LIB_ENABLE_IOSTREAM)

# Add all CPP files to the executable
add_executable(${PROJECT_NAME} src/main.cpp src/RobotBase.cpp src/drivetrain.cpp src/deposition_auto.cpp src/linear_actuators.cpp)

# Link against required libraries
target_include_directories(${PROJECT_NAME} PUBLIC
  ${sensor_msgs_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  phoenix6
  Threads::Threads
  ${SDL2_LIBRARIES}
  rclcpp::rclcpp
  ${sensor_msgs_LIBRARIES}
  rosidl_typesupport_cpp::rosidl_typesupport_cpp
  rosidl_typesupport_fastrtps_cpp::rosidl_typesupport_fastrtps_cpp
)