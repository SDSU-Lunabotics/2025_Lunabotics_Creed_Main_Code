#include "deposition.hpp"

Deposition::Deposition() 
  : Node("deposition_node"),
    latest_joy_received_(false),
    collector_button_prev_(false), collector_toggle_state_(false),
    dump_button_prev_(false), dump_toggle_state_(false)
{
    // Subscribe to joystick messages (using the raw "joy" topic or a filtered topic if desired).
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10,
        std::bind(&Deposition::joyCallback, this, std::placeholders::_1));

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
    // Set the duty cycle for each motor independently based on its toggle state.
    if (collector_toggle_state_) {
        collectorOutput_.Output = -0.5;
    } else {
        collectorOutput_.Output = 0.0;
    }
    if (dump_toggle_state_) {
        dumpOutput_.Output = 0.5;
    } else {
        dumpOutput_.Output = 0.0;
    }
    // Apply commands to each motor.
    regolith_collector_.SetControl(collectorOutput_);
    regolith_dump_.SetControl(dumpOutput_);
}

void Deposition::DisabledInit() {
    regolith_collector_.SetControl(controls::NeutralOut{});
    regolith_dump_.SetControl(controls::NeutralOut{});
}

void Deposition::DisabledPeriodic() {
    // Nothing required here.
}

void Deposition::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    // Ensure the message contains at least two buttons.
    if (msg->buttons.size() < 2) return;

    // For the regolith collector (button 0)
    bool collector_button_current = (msg->buttons[0] == 1);
    if (collector_button_current && !collector_button_prev_) {
        collector_toggle_state_ = !collector_toggle_state_;
        RCLCPP_INFO(this->get_logger(), "Regolith Collector toggled: %s",
                    collector_toggle_state_ ? "ON" : "OFF");
    }
    collector_button_prev_ = collector_button_current;

    // For the regolith dump (button 1)
    bool dump_button_current = (msg->buttons[1] == 1);
    if (dump_button_current && !dump_button_prev_) {
        dump_toggle_state_ = !dump_toggle_state_;
        RCLCPP_INFO(this->get_logger(), "Regolith Dump toggled: %s",
                    dump_toggle_state_ ? "ON" : "OFF");
    }
    dump_button_prev_ = dump_button_current;
}
