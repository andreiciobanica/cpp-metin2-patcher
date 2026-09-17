#include "label_minimize.h"

label_minimize::label_minimize(QWidget* parent) : QLabel(parent)
{

}

void label_minimize::mousePressEvent(QMouseEvent* ev)
{
    emit Mouse_Pressed_Minimize();
}
