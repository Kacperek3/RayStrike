#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include <memory>

#include "State.h"

typedef std::unique_ptr<State> StateRef;

class GameStateManager {
public:
    GameStateManager() = default;
    ~GameStateManager() = default;

    void AddState(StateRef newState, bool isReplacing = true);
    void RemoveState();
    void ProcessStateChanges();
    StateRef &GetActiveState();
    

private:
    std::stack<StateRef> _states;
    StateRef _newState;

    bool _isRemoving;
    bool _isAdding;
    bool _isReplacing;
};
