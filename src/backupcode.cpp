#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ctre/phoenix6/TalonFX.hpp"
#include "RobotBase.hpp"
#include <thread>
#include <cmath>

using namespace ctre::phoenix6;

class Robot : public RobotBase, public rclcpp::Node {
public:
    Robot() : Node("robot_node"), latest_joy_received_(false) {
        subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10,
            std::bind(&Robot::joyCallback, this, std::placeholders::_1));
    }

    void RobotInit() override {
        configs::TalonFXConfiguration fx_cfg{};
        
        // Configure back left motor (leader)
        fx_cfg.MotorOutput.Inverted = signals::InvertedValue::CounterClockwise_Positive;
        fx_cfg.CurrentLimits.SupplyCurrentLimitEnable = true;
        fx_cfg.CurrentLimits.SupplyCurrentLimit = units::current::ampere_t(5.17);
        
        auto resultBack = leftBackMotor_.GetConfigurator().Apply(fx_cfg);
        if (!resultBack.IsOK()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to configure left back motor: %s", resultBack.GetDescription());
        } else {
            RCLCPP_INFO(this->get_logger(), "Successfully configured left back motor");
        }

        // Configure front left motor (follower)
        auto resultFront = leftFrontMotor_.GetConfigurator().Apply(fx_cfg);
        if (!resultFront.IsOK()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to configure left front motor: %s", resultFront.GetDescription());
        } else {
            RCLCPP_INFO(this->get_logger(), "Successfully configured left front motor");
        }

        // Set front motor to follow back motor
        leftFrontMotor_.SetControl(controls::Follower{leftBackMotor_.GetDeviceID(), false});
        RCLCPP_INFO(this->get_logger(), "Set left front motor to follow left back motor");
    }

    void RobotPeriodic() override {
        // You can add any periodic tasks here.
    }

    bool IsEnabled() override {
        return latest_joy_received_;
    }

    void EnabledInit() override {
        RCLCPP_INFO(this->get_logger(), "Robot ENABLED");
    }

    void EnabledPeriodic() override {
        if (latest_joy_received_) {
            if (latest_joy_msg_.axes.size() > 4) {
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
        // Only need to control the leader motor, follower will automatically match
        leftBackMotor_.SetControl(motorOut_);
    }

    void DisabledInit() override {
        RCLCPP_INFO(this->get_logger(), "Robot DISABLED");
    }

    void DisabledPeriodic() override {
        leftBackMotor_.SetControl(controls::NeutralOut{});
    }

private:
    void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        latest_joy_msg_ = *msg;
        latest_joy_received_ = true;
    }

    static constexpr char const *CANBUS_NAME = "can0";
    // Left back motor (leader) with ID 1
    hardware::TalonFX leftBackMotor_{1, CANBUS_NAME};
    // Left front motor (follower) with ID 2
    hardware::TalonFX leftFrontMotor_{5, CANBUS_NAME};
    controls::DutyCycleOut motorOut_{0};
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    sensor_msgs::msg::Joy latest_joy_msg_;
    bool latest_joy_received_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto robot = std::make_shared<Robot>();

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