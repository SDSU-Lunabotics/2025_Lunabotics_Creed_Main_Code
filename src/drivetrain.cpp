#include "drivetrain.hpp"

DriveTrain::DriveTrain()
  : Node("drivetrain_node"), latest_joy_received_(false)
{
    // Subscribe to the joy topic.
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10,
        std::bind(&DriveTrain::joyCallback, this, std::placeholders::_1));

                // Create a timer that calls EnabledPeriodic() every 20 milliseconds.
    // timer_ = this->create_wall_timer(
    //     std::chrono::milliseconds(20),
    //     std::bind(&DriveTrain::EnabledPeriodic, this)
    // );
}

void DriveTrain::RobotInit() {
    configs::TalonFXConfiguration fx_cfg{};
    fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
    fx_cfg.CurrentLimits.SupplyCurrentLimitEnable = true;
    fx_cfg.CurrentLimits.SupplyCurrentLimit = units::current::ampere_t(10);

    // Configure left side motors.
    auto resultLeftBack = leftBackMotor_.GetConfigurator().Apply(fx_cfg);
    auto resultLeftFront = leftFrontMotor_.GetConfigurator().Apply(fx_cfg);
    if (!resultLeftBack.IsOK() || !resultLeftFront.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure left motors");
    } else {
        RCLCPP_INFO(this->get_logger(), "Left motors configured");
    }
    // Set left front motor to follow left back motor.
    leftFrontMotor_.SetControl(controls::Follower{leftBackMotor_.GetDeviceID(), false});
    
    // Configure right side motors.
    auto resultRightBack = rightBackMotor_.GetConfigurator().Apply(fx_cfg);
    auto resultRightFront = rightFrontMotor_.GetConfigurator().Apply(fx_cfg);
    if (!resultRightBack.IsOK() || !resultRightFront.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to right configure motors");
    } else {
        RCLCPP_INFO(this->get_logger(), "Right motors configured");
    }
    // Set right front motor to follow right back motor.
    rightFrontMotor_.SetControl(controls::Follower{rightBackMotor_.GetDeviceID(), false});
}

void DriveTrain::RobotPeriodic() {
    // Add any periodic tasks if needed.
}

bool DriveTrain::IsEnabled() {
    return latest_joy_received_;
}

void DriveTrain::EnabledInit() {
    RCLCPP_INFO(this->get_logger(), "DriveTrain ENABLED");
}

void DriveTrain::EnabledPeriodic() {
    if (latest_joy_received_) {
        if (latest_joy_msg_.axes.size() > 4) {
            // Left side: use left stick vertical (e.g. axis 1).
            double left_speed = -latest_joy_msg_.axes[1];
            // Right side: use right stick vertical (e.g. axis 4).
            double right_speed = -latest_joy_msg_.axes[4];
            
            leftMotorOut_.Output = left_speed;
            rightMotorOut_.Output = right_speed;
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                                 "Not enough joystick axes");
            leftMotorOut_.Output = 0.0;
            rightMotorOut_.Output = 0.0;
        }
    } else {
        leftMotorOut_.Output = 0.0;
        rightMotorOut_.Output = 0.0;
    }
    // Send commands to the leader motors.
    leftBackMotor_.SetControl(leftMotorOut_);
    rightBackMotor_.SetControl(rightMotorOut_);
}

void DriveTrain::DisabledInit() {
    RCLCPP_INFO(this->get_logger(), "DriveTrain DISABLED");
}

void DriveTrain::DisabledPeriodic() {
    leftBackMotor_.SetControl(controls::NeutralOut{});
    rightBackMotor_.SetControl(controls::NeutralOut{});
}

void DriveTrain::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    latest_joy_msg_ = *msg;
    latest_joy_received_ = true;
}