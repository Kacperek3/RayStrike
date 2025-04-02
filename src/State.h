#pragma once
#include <SFML/Graphics.hpp>

class State {
public:
    virtual void Init() = 0;    
    virtual void HandleInput() = 0;  
    virtual void Update() = 0;       
    virtual void Draw() = 0;  
    virtual void Pause() {};
    virtual void Resume() {};      
    virtual void ClearObjects() {};      
};
