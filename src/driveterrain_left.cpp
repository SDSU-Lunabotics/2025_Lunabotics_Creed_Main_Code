#include "driveterrain.hpp"

DriveTerrain::DriveTerrain() 
  : Node("robot_node"), latest_joy_received_(false)
{
    // Create the subscription for the joy topic
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10,
        std::bind(&DriveTerrain::joyCallback, this, std::placeholders::_1));
}

void DriveTerrain::RobotInit() {
    configs::TalonFXConfiguration fx_cfg{};

    // Configure left back motor (leader)
    fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
    fx_cfg.CurrentLimits.SupplyCurrentLimitEnable = true;
    fx_cfg.CurrentLimits.SupplyCurrentLimit = units::current::ampere_t(5.17);
    
    auto resultBack = leftBackMotor_.GetConfigurator().Apply(fx_cfg);
    if (!resultBack.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure left back motor: %s", resultBack.GetDescription());
    } else {
        RCLCPP_INFO(this->get_logger(), "Successfully configured left back motor");
    }

    // Configure left front motor (follower)
    auto resultFront = leftFrontMotor_.GetConfigurator().Apply(fx_cfg);
    if (!resultFront.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure left front motor: %s", resultFront.GetDescription());
    } else {
        RCLCPP_INFO(this->get_logger(), "Successfully configured left front motor");
    }

    // Set the front motor to follow the back motor
    leftFrontMotor_.SetControl(controls::Follower{leftBackMotor_.GetDeviceID(), false});
    RCLCPP_INFO(this->get_logger(), "Set left front motor to follow left back motor");
}

void DriveTerrain::RobotPeriodic() {
    // Add periodic tasks here if needed.
}

bool DriveTerrain::IsEnabled() {
    return latest_joy_received_;
}

void DriveTerrain::EnabledInit() {
    RCLCPP_INFO(this->get_logger(), "Robot ENABLED");
}

void DriveTerrain::EnabledPeriodic() {
    if (latest_joy_received_) {
        if (latest_joy_msg_.axes.size() > 1) {
            double speed = -latest_joy_msg_.axes[1];  // Use left stick vertical for forward/backward
            motorOut_.Output = speed;
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                "Not enough joystick axes");
            motorOut_.Output = 0.0;
        }
    } else {
        motorOut_.Output = 0.0;
    }
    // Only control the leader motor; the follower will mirror it.
    leftBackMotor_.SetControl(motorOut_);
}

void DriveTerrain::DisabledInit() {
    RCLCPP_INFO(this->get_logger(), "Robot DISABLED");
}

void DriveTerrain::DisabledPeriodic() {
    leftBackMotor_.SetControl(controls::NeutralOut{});
}

void DriveTerrain::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    latest_joy_msg_ = *msg;
    latest_joy_received_ = true;
}
