#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include "precomp.h"
#include "mouse.h"
#include "screenSizeTransfer.h"

class Button
{
    private:
        std::string label;
        rl::Rectangle bounds;
        //Returns a rectangle that surrounds the text
        rl::Rectangle labelBounds(const std::string& text, int font, int posX, int posY);
    public:
        Button(const std::string& label, int font, int posX, int posY);
        Button(const std::string& label, rl::Rectangle buttonDimensions);
        bool isHovered(const Mouse& mouse) const;
        const std::string& getLabel() const;
        const rl::Rectangle& getBounds() const;
        //Used for moving objects
        void updateButtonBounds(rl::Rectangle newButtonDimensions);
};

#endif