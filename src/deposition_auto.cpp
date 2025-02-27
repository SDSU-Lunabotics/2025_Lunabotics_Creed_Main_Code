#include "deposition.hpp"

Deposition::Deposition() 
  : Node("deposition_node"), button_a_prev_(false), toggle_state_(false)
{
    // Subscribe to joystick messages.
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10,
        std::bind(&Deposition::joyCallback, this, std::placeholders::_1));

        // Create a timer that calls EnabledPeriodic() every 20 milliseconds.
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(20),
        std::bind(&Deposition::EnabledPeriodic, this)
    );
}

void Deposition::RobotInit() {
    configs::TalonFXConfiguration cfg{};
    cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
    auto res1 = regolith_collector_.GetConfigurator().Apply(cfg);
    auto res2 = regolith_dump_.GetConfigurator().Apply(cfg);
    if (!res1.IsOK() || !res2.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure deposition motors");
    } else {
        RCLCPP_INFO(this->get_logger(), "Deposition motors configured");
    }
}

void Deposition::RobotPeriodic() {
    // No extra periodic tasks for deposition.
}

bool Deposition::IsEnabled() {
    return true;
}

void Deposition::EnabledInit() {
    RCLCPP_INFO(this->get_logger(), "Deposition module ENABLED");
}

void Deposition::EnabledPeriodic() {
    // Toggle motor output based on the internal toggle state.
    if (toggle_state_) {
        // Run motors at a set duty cycle ( 50% output);
        depositionMotorOut_.Output = 0.5;
    } else {
        depositionMotorOut_.Output = 0.0;
    }
    // Set both motors with the same control command.
    regolith_collector_.SetControl(depositionMotorOut_);
    regolith_dump_.SetControl(depositionMotorOut_);
}

void Deposition::DisabledInit() {
    regolith_collector_.SetControl(controls::NeutralOut{});
    regolith_dump_.SetControl(controls::NeutralOut{});
}

void Deposition::DisabledPeriodic() {
    // Nothing required here.
}

void Deposition::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    bool button_a_current = false;
    if (!msg->buttons.empty()) {
        button_a_current = (msg->buttons[0] == 1);  // Assuming button A is at index 0. need to test first !!!!

    }
    // Rising-edge detection: if button is pressed now but wasn't before, toggle.
    if (button_a_current && !button_a_prev_) {
        toggle_state_ = !toggle_state_;
        RCLCPP_INFO(this->get_logger(), "Deposition motors toggled: %s", toggle_state_ ? "ON" : "OFF");
    }
    button_a_prev_ = button_a_current;
}

