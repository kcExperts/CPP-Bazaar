#include "button.h"

rl::Rectangle Button::labelBounds(const std::string& text, int font, int posX, int posY)
{
    float rectX = sst::cx(posX);
    float rectY = sst::cy(posY);
    float width = rl::MeasureText(text.c_str(), sst::cx(font));
    rl::Rectangle rec = {rectX, rectY, width, (float)sst::cy(font)};
    return rec;
}

Button::Button(const std::string& label, int font, int posX, int posY)
{
    this->label = label;
    bounds = labelBounds(this->label, font, posX, posY);
}

Button::Button(const std::string& label, rl::Rectangle buttonDimensions)
{
    this->label = label;
    bounds = {(float)sst::cx(buttonDimensions.x), (float)sst::cy(buttonDimensions.y), (float)sst::cx(buttonDimensions.width), (float)sst::cy(buttonDimensions.height)};
}

bool Button::isHovered(const Mouse& mouse) const
{
    return CheckCollisionRecs(mouse.mouseHitbox(), bounds);
}

const std::string& Button::getLabel() const
{
    return label;
}

const rl::Rectangle& Button::getBounds() const
{
    return bounds;
}

void Button::updateButtonBounds(rl::Rectangle newButtonDimensions)
{
    bounds = {(float)sst::cx(newButtonDimensions.x), (float)sst::cy(newButtonDimensions.y), (float)sst::cx(newButtonDimensions.width), (float)sst::cy(newButtonDimensions.height)};
}