#ifndef MOUSE_H
#define MOUSE_H

#include "precomp.h"

class Mouse
{
    private:
        rl::Vector2 mousePos;
        rl::Rectangle mouse;
    public:
        //Creates mouse and pos for mouse hitbox
        Mouse();
        //Returns a const reference containing the position of the mouse
        const rl::Vector2& position() const;
        //Retuns a constant reference to the mouse hitbox
        const rl::Rectangle& mouseHitbox() const;
        //Updates the position of the mouse
        void updateMousePosition();
};

#endif