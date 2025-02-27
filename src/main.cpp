#include "rclcpp/rclcpp.hpp"
#include "driveterrain.hpp"
#include "deposition.hpp"
#include "RobotBase.hpp"
#include <atomic>
#include <memory>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    // Create the DriveTerrain object.
    // that also inherits from RobotBase.
    auto driveterrain = std::make_shared<DriveTerrain>();
    auto deposition = std::make_shared<Deposition>();

    // Spin the ROS 2 node to process callbacks (like joystick messages).
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(driveterrain);
    executor.add_node(deposition);
    executor.spin();

    rclcpp::shutdown();
    return 0;
}
