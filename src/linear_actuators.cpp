// #include "linear_actuators.hpp"
// #include <chrono>
// using namespace std::chrono_literals;

// LinearActuators::LinearActuators()
//   : Node("linear_actuators_node"), latest_joy_received_(false)
// {
//     // Subscribe to the "joy" topic.
//     subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
//         "joy", 10,
//         std::bind(&LinearActuators::joyCallback, this, std::placeholders::_1));

//     // If using a manual Run() loop (like in your LinearActuators code), you don't need a timer.
//     // Otherwise, you could set up a timer to call EnabledPeriodic() periodically.

//     // Call initialization functions once.
//     RobotInit();
//     EnabledInit();
// }

// void LinearActuators::RobotInit() {
//     configs::TalonFXConfiguration fx_cfg{};
//     fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
//     fx_cfg.CurrentLimits.SupplyCurrentLimitEnable = true;
//     fx_cfg.CurrentLimits.SupplyCurrentLimit = units::current::ampere_t(10); // example limit

//     auto resLeader = leaderMotor_.GetConfigurator().Apply(fx_cfg);
//     auto resFollower = followerMotor_.GetConfigurator().Apply(fx_cfg);
//     if (!resLeader.IsOK() || !resFollower.IsOK()) {
//         RCLCPP_ERROR(this->get_logger(), "Failed to configure linear actuator motors");
//     } else {
//         RCLCPP_INFO(this->get_logger(), "Linear actuator motors configured");
//     }
//     // Set follower to follow the leader.
//     followerMotor_.SetControl(controls::Follower{leaderMotor_.GetDeviceID(), false});
// }

// void LinearActuators::RobotPeriodic() {
//     // Any additional periodic tasks can be added here.
// }

// bool LinearActuators::IsEnabled() {
//     return latest_joy_received_;
// }

// void LinearActuators::EnabledInit() {
//     RCLCPP_INFO(this->get_logger(), "Linear Actuators ENABLED");
// }

// void LinearActuators::EnabledPeriodic() {
//     double command = 0.0;
//     // Check that the joystick message has enough buttons.
//     if (latest_joy_received_ && latest_joy_msg_.buttons.size() > 6) {
//         // Instead of using axes, we simply use the button state.
//         // For example:
//         // If button 6 is pressed, command upward motion (constant positive output).
//         if (latest_joy_msg_.buttons[6] == 1) {
//             command = 1.0;  // full upward power (adjust as needed)
//         }
//         // If button 5 is pressed, command downward motion.
//         else if (latest_joy_msg_.buttons[5] == 1) {
//             command = -1.0; // full downward power (adjust as needed)
//         }
//         else {
//             command = 0.0;
//         }
//     } else {
//         command = 0.0;
//     }
//     actuatorOutput_.Output = command;
//     // Command the leader motor; follower automatically follows.
//     leaderMotor_.SetControl(actuatorOutput_);
// }

// void LinearActuators::DisabledInit() {
//     RCLCPP_INFO(this->get_logger(), "Linear Actuators DISABLED");
//     leaderMotor_.SetControl(controls::NeutralOut{});
// }

// void LinearActuators::DisabledPeriodic() {
//     // No additional actions needed.
// }

// void LinearActuators::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
//     latest_joy_msg_ = *msg;
//     latest_joy_received_ = true;
// }

#include "linear_actuators.hpp"
#include <chrono>
using namespace std::chrono_literals;

LinearActuators::LinearActuators()
  : Node("linear_actuators_node"), latest_joy_received_(false)
{
    // Subscribe to the "joy" topic.
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10,
        std::bind(&LinearActuators::joyCallback, this, std::placeholders::_1));

    // Call initialization methods once (assuming you're using a manual Run() loop).
    RobotInit();
    EnabledInit();
}

void LinearActuators::RobotInit() {
    configs::TalonFXConfiguration fx_cfg{};
    fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
    fx_cfg.CurrentLimits.SupplyCurrentLimitEnable = true;
    fx_cfg.CurrentLimits.SupplyCurrentLimit = units::current::ampere_t(10); // Example current limit

    auto resLeader = leaderMotor_.GetConfigurator().Apply(fx_cfg);
    auto resFollower = followerMotor_.GetConfigurator().Apply(fx_cfg);
    if (!resLeader.IsOK() || !resFollower.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure linear actuator motors");
    } else {
        RCLCPP_INFO(this->get_logger(), "Linear actuator motors configured");
    }
    // Set follower to follow the leader.
    followerMotor_.SetControl(controls::Follower{leaderMotor_.GetDeviceID(), false});
}

void LinearActuators::RobotPeriodic() {
    // Optional: add any periodic tasks if needed.
}

bool LinearActuators::IsEnabled() {
    return latest_joy_received_;
}

void LinearActuators::EnabledInit() {
    RCLCPP_INFO(this->get_logger(), "Linear Actuators ENABLED");
}

void LinearActuators::EnabledPeriodic() {
    double command = 0.0;
    // Ensure the joystick message has enough axes (need index 3 and 6).
    if (latest_joy_received_ && latest_joy_msg_.axes.size() > 6) {
        // Use RT (axis 3) and LT (axis 6) to determine the command.
        double rt_value = latest_joy_msg_.axes[3];
        double lt_value = latest_joy_msg_.axes[6];
        // Compute command as difference. Adjust signs as needed.
        command = rt_value - lt_value;
    }
    actuatorOutput_.Output = command;
    // Command the leader motor. The follower will mirror automatically.
    leaderMotor_.SetControl(actuatorOutput_);
}

void LinearActuators::DisabledInit() {
    RCLCPP_INFO(this->get_logger(), "Linear Actuators DISABLED");
    leaderMotor_.SetControl(controls::NeutralOut{});
}

void LinearActuators::DisabledPeriodic() {
    // No additional actions required.
}

void LinearActuators::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    // Make sure there are enough axes before using them.
    if (msg->axes.size() <= 6) return;
    latest_joy_msg_ = *msg;
    latest_joy_received_ = true;
}