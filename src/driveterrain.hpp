#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"  // Your existing RobotBase interface
#include <cmath>

using namespace ctre::phoenix6;

class DriveTerrain : public RobotBase, public rclcpp::Node {
public:
    DriveTerrain();

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

    static constexpr char const *CANBUS_NAME = "can0";
    // Left back motor (leader) with ID 1
    hardware::TalonFX leftBackMotor_{1, CANBUS_NAME};
    // Left front motor (follower) with ID 5
    hardware::TalonFX leftFrontMotor_{5, CANBUS_NAME};
    controls::DutyCycleOut motorOut_{0};
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    sensor_msgs::msg::Joy latest_joy_msg_;
    bool latest_joy_received_;
};
