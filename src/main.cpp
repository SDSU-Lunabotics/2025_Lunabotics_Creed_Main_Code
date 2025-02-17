#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"
#include <thread>
#include <cmath>

using namespace ctre::phoenix6;

/**
 * The Robot class now inherits from both RobotBase and rclcpp::Node.
 * It subscribes to joystick messages on the "/joy" topic.
 */
class Robot : public RobotBase, public rclcpp::Node {
public:
  Robot() : Node("robot_node"), latest_joy_received_(false) {
    // Create the subscription to the /joy topic.
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
      "joy", 10,
      std::bind(&Robot::joyCallback, this, std::placeholders::_1));
  }

  // --- RobotBase interface methods ---

  void RobotInit() override {
    configs::TalonFXConfiguration fx_cfg{};
    fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
    auto result = motor_.GetConfigurator().Apply(fx_cfg);
    if (!result.IsOK()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to configure motor: %s", result.GetDescription());  // Removed .c_str()
    }
  }

  void RobotPeriodic() override {
    // You can add any periodic tasks here.
  }

  bool IsEnabled() override {
    // For this example, we enable if we've received a joystick message.
    return latest_joy_received_;
  }

  void EnabledInit() override {
    RCLCPP_INFO(this->get_logger(), "Robot ENABLED");
  }



void EnabledPeriodic() override {
    if (latest_joy_received_) {
        // Ensure we have enough axes (at least 5 axes: 0-4)
        if (latest_joy_msg_.axes.size() > 4) {
            // Use right stick vertical (e.g., axis 4) for forward/backward motion.
            // Invert the value if needed.
            double speed = -latest_joy_msg_.axes[4];
            motorOut_.Output = speed;
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                                   "Not enough joystick axes");
            motorOut_.Output = 0.0;
        }
    } else {
        motorOut_.Output = 0.0;
    }
    motor_.SetControl(motorOut_);
}




  void DisabledInit() override {
    RCLCPP_INFO(this->get_logger(), "Robot DISABLED");
  }

  void DisabledPeriodic() override {
    motor_.SetControl(controls::NeutralOut{});
  }

  // --- End of RobotBase interface ---

private:
  // Callback for receiving joystick messages.
  void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    latest_joy_msg_ = *msg;
    latest_joy_received_ = true;
  }

  // Robot components
  static constexpr char const *CANBUS_NAME = "can0";
  hardware::TalonFX motor_{5, CANBUS_NAME};
  controls::DutyCycleOut motorOut_{0};

  // ROS2 subscription members
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
  sensor_msgs::msg::Joy latest_joy_msg_;
  bool latest_joy_received_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto robot = std::make_shared<Robot>();
    
    // Handle shutdown gracefully
    std::atomic<bool> running{true};
    std::thread robotThread([&](){
        while(running && rclcpp::ok()) {
            robot->Run();
        }
    });
    
    rclcpp::spin(robot);
    running = false;
    robotThread.join();
    rclcpp::shutdown();
    return 0;
}
