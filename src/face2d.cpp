#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>

#include "rclcpp/rclcpp.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#define INOCHI2D_GLYES
#include <inochi2d.h>

double getTime()
{
    return static_cast<double>(sf::Clock().getElapsedTime().asSeconds());
}

std::atomic<bool> running { true };

void render_loop()
{
    sf::Window window { sf::VideoMode { {800, 600} }, "Hello World!" };
    window.setVerticalSyncEnabled(true);

    inInit(getTime);
    inViewportSet(800, 600);

    InPuppet* puppet = inPuppetLoad("/home/tomicodev/projects3/ros2_expression_ws/src/ros2_expression/face2d/puppet/maru_face.inx");
    if (!puppet)
    {
        std::cout << "Failed to load puppet!\n";
        running = false;
    } else {
        std::cout << "Loaded puppet!\n";
    }

    InCamera* cam = inCameraGetCurrent();
    inCameraSetZoom(cam, 0.8f);            // Adjust zoom level if needed
    inCameraSetPosition(cam, 0.0f, 0.0f);  // Center the camera


    while (running and window.isOpen()) 
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                running = false;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        inUpdate();
        inSceneBegin();
        inPuppetUpdate(puppet);
        inPuppetDraw(puppet);
        inSceneEnd();
        inSceneDraw(0, 0, 800, 600);
        
        
        window.display();
    }

    inPuppetDestroy(puppet);
    inCleanup();
}

class Face2DNode : public rclcpp::Node
{
public:
    Face2DNode()
    : Node("face2d")
    {
        RCLCPP_INFO(get_logger(), "Hello world!");
    }
}; 

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    std::thread renderThread { render_loop };

    rclcpp::spin(std::make_shared<Face2DNode>());

    // shutdown
    running = false;
    if (renderThread.joinable())
        renderThread.join();
    
    rclcpp::shutdown();
    return 0;
}