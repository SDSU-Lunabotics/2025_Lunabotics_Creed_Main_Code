#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"  
#include "constants.hpp"
#include <cmath>
#include <chrono>

using namespace ctre::phoenix6;

class Deposition : public RobotBase, public rclcpp::Node {
public:
    Deposition();

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

    hardware::TalonFX regolith_collector_{5, CANBUS_NAME};
    hardware::TalonFX regolith_dump_{14, CANBUS_NAME};

    controls::DutyCycleOut depositionMotorOut_{0};
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    sensor_msgs::msg::Joy latest_joy_msg_;
    bool latest_joy_received_;
    bool button_a_prev_;  // tracks previous button state for rising-edge detection
    bool toggle_state_;   // false = motors off, true = motors on

    rclcpp::TimerBase::SharedPtr timer_;
};