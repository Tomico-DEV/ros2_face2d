#include "face2d/puppet_window.hpp"


namespace face2d
{

PuppetWindow::PuppetWindow(std::string name, sf::Vector2u size, uint32_t style)
: window_name_ { name }, size_ { size }, style_ { style }
{ }

void PuppetWindow::start()
{
    running_ = true;
    window_thread_ = std::thread(&PuppetWindow::update_window_, this);
}

void PuppetWindow::stop()
{
    running_ = false;
    if (window_thread_.joinable())
    {
        window_thread_.join();
    }

}

void PuppetWindow::update_window_()
{
    window_.create(sf::VideoMode { size_ }, window_name_, style_); 
    window_.setVerticalSyncEnabled(true);

    inInit([]() -> double { 
        return static_cast<double>(sf::Clock().getElapsedTime().asSeconds()); 
    });

    {
        std::lock_guard<std::mutex> lock { window_mutex_ };
        if (!p_puppet_)
            p_puppet_ = inPuppetLoad(puppet_filepath_.c_str());
    }

    inViewportSet(size_.x, size_.y);

    InCamera* cam = inCameraGetCurrent();
    inCameraSetZoom(cam, 0.8f);            // Adjust zoom level if needed
    inCameraSetPosition(cam, 0.0f, 0.0f);  // Center the camera

    while (running_)
    {
        
        while (const std::optional event = window_.pollEvent())
        {
            
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
                running_ = false;
            }
        
            if (const auto * resized = event->getIf<sf::Event::Resized>())
            {
                size_.x = resized->size.x;
                size_.y = resized->size.y;
                glViewport(0, 0, size_.x, size_.y);
                inViewportSet(size_.x, size_.y);
            }
        }
        
        
        (void) window_.setActive(true);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        {
            std::lock_guard<std::mutex> lock { window_mutex_ };
            if (p_puppet_)
            {
                inUpdate();
                inSceneBegin();
                inPuppetUpdate(p_puppet_);
                inPuppetDraw(p_puppet_);
                inSceneEnd();
                inSceneDraw(0, 0, size_.x, size_.y);
            }
        }

        window_.display();
    }

    inPuppetDestroy(p_puppet_);
    // unload inochi2d runtime
    inCleanup();
}

void PuppetWindow::set_puppet(const std::string& fpath)
{
    std::lock_guard<std::mutex> lock { window_mutex_ };
    if (std::filesystem::exists(fpath))
        puppet_filepath_ = fpath;
    else
        throw std::runtime_error("Provided file does not exist!");
}

void PuppetWindow::reload_puppet()
{
    std::lock_guard<std::mutex> lock { window_mutex_ };
    if (running_)
    {
        (void) window_.setActive(true);
        if (p_puppet_)
            inPuppetDestroy(p_puppet_);
        p_puppet_ = inPuppetLoad(puppet_filepath_.c_str());
    }
    
}

}
