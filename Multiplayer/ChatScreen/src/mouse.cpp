#include "mouse.h"

Mouse::Mouse()
{
    mouse = {rl::GetScreenWidth()/2.0f - 30, rl::GetScreenHeight()/2.0f - 30, 5, 5};
}

void Mouse::updateMousePosition()
{
    mousePos.x = rl::GetMouseX();
    mousePos.y = rl::GetMouseY();
    mouse.x = mousePos.x - mouse.width/2;
    mouse.y = mousePos.y - mouse.height/2;
}

const rl::Rectangle& Mouse::mouseHitbox() const
{
    return mouse;
}

const rl::Vector2& Mouse::position() const
{
    return mousePos;
}