#include "rclcpp/rclcpp.hpp"
#include "driveterrain.hpp"
#include "RobotBase.hpp"
#include <thread>
#include <atomic>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    // Create the DriveTerrain object.
    // In the future, you could switch this out for another module (e.g., RegolithMaterial)
    // that also inherits from RobotBase.
    auto robot = std::make_shared<DriveTerrain>();

    // Run the robot control loop in a separate thread.
    std::atomic<bool> running{true};
    std::thread robotThread([&](){
        while(running && rclcpp::ok()) {
            robot->Run();
        }
    });

    // Spin the ROS 2 node to process callbacks (like joystick messages).
    rclcpp::spin(robot);
    running = false;
    robotThread.join();
    rclcpp::shutdown();
    return 0;
}
