#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"
#include "constants.hpp"  // For CANBUS_NAME macro, e.g. #define CANBUS_NAME "can0"
#include <chrono>

using namespace ctre::phoenix6;

class LinearActuators : public RobotBase, public rclcpp::Node {
public:
    LinearActuators();

    // RobotBase interface methods:
    void RobotInit() override;
    void RobotPeriodic() override;
    bool IsEnabled() override;
    void EnabledInit() override;
    void EnabledPeriodic() override;
    void DisabledInit() override;
    void DisabledPeriodic() override;

private:
    void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg);

    // Linear actuator motors: one leader and one follower.
    hardware::TalonFX leaderMotor_{15, CANBUS_NAME};  
    hardware::TalonFX followerMotor_{16, CANBUS_NAME};   

    // Duty cycle output to command the actuator.
    controls::DutyCycleOut actuatorOutput_{0};

    // Joystick subscription and storage.
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    sensor_msgs::msg::Joy latest_joy_msg_;
    bool latest_joy_received_;
};
