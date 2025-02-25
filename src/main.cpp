#include "rclcpp/rclcpp.hpp"
#include "driveterrain.hpp"
#include "deposition.hpp"
#include "RobotBase.hpp"
#include <thread>
#include <atomic>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    // Create the DriveTerrain object.
    // In the future, you could switch this out for another module (e.g., RegolithMaterial)
    // that also inherits from RobotBase.
    auto driveterrain = std::make_shared<DriveTerrain>();
    auto deposition = std::make_shared<Deposition>();


    // Run the robot control loop in a separate thread.
    std::atomic<bool> running{true};
    std::thread robotThread([&](){
        while(running && rclcpp::ok()) {
            driveterrain->Run();
            deposition->Run();
        }
    });

    // Spin the ROS 2 node to process callbacks (like joystick messages).
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(driveterrain);
    executor.add_node(deposition);
    executor.spin();

    running = false;
    robotThread.join();
    rclcpp::shutdown();
    return 0;
}
