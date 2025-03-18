// #include "rclcpp/rclcpp.hpp"
// #include "drivetrain.hpp"
// #include "deposition.hpp"
// #include "RobotBase.hpp"
// #include <atomic>
// #include <memory>

// int main(int argc, char **argv) {
//     rclcpp::init(argc, argv);

//     // Create the DriveTrain object.
//     // that also inherits from RobotBase.
//     auto drivetrain = std::make_shared<DriveTrain>();
//     auto deposition = std::make_shared<Deposition>();

//     // Spin the ROS 2 node to process callbacks (like joystick messages).
//     rclcpp::executors::MultiThreadedExecutor executor;
//     executor.add_node(drivetrain);
//     executor.add_node(deposition);
//     executor.spin();

//     rclcpp::shutdown();
//     return 0;
// }


#include "rclcpp/rclcpp.hpp"
#include "drivetrain.hpp"
#include "deposition.hpp"
#include "linear_actuators.hpp"
#include "RobotBase.hpp"
#include <thread>
#include <atomic>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    auto driveTrain = std::make_shared<DriveTrain>();
    auto deposition   = std::make_shared<Deposition>();
    auto linear_actuator   = std::make_shared<LinearActuators>();


    // Create separate threads for spinning each node:
    std::thread driveSpinThread([&](){
        rclcpp::spin(driveTrain);
    });

    std::thread depositionSpinThread([&](){
        rclcpp::spin(deposition);
    });

    std::thread actuatorSpinThread([&](){
        rclcpp::spin(linear_actuator);
    });

    // (Assuming your RobotBase::Run() handles initialization and periodic tasks.)
    std::thread driveRunThread([&](){
        driveTrain->Run();
    });

    std::thread depositionRunThread([&](){
        deposition->Run();
    });

    std::thread actuatorRunThread([&](){
        linear_actuator->Run();
    });


    // Wait for all threads to finish (or use a mechanism to stop them on shutdown).
    driveSpinThread.join();
    depositionSpinThread.join();
    driveRunThread.join();
    depositionRunThread.join();
    actuatorSpinThread.join();
    actuatorRunThread.join();


    rclcpp::shutdown();
    return 0;
}
