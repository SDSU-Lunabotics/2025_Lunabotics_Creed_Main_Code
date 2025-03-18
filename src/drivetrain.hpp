#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"
#include "constants.hpp" 
#include <cmath>
#include <chrono>

using namespace ctre::phoenix6;

class DriveTrain : public RobotBase, public rclcpp::Node {
public:
    DriveTrain();

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

    // Left side motors (leader and follower)
    hardware::TalonFX leftBackMotor_{5, CANBUS_NAME};   // leader
    hardware::TalonFX leftFrontMotor_{1, CANBUS_NAME};  // follower

    // Right side motors (leader and follower)
    hardware::TalonFX rightBackMotor_{11, CANBUS_NAME};   // leader 
    hardware::TalonFX rightFrontMotor_{2, CANBUS_NAME};  // follower 

    // duty cycle outputs for left and right sides
    controls::DutyCycleOut leftMotorOut_{0};
    controls::DutyCycleOut rightMotorOut_{0};
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    sensor_msgs::msg::Joy latest_joy_msg_;
    bool latest_joy_received_;

   //rclcpp::TimerBase::SharedPtr timer_;
};