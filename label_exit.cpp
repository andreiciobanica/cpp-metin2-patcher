#include "label_exit.h"

label_exit::label_exit(QWidget* parent) : QLabel(parent)
{

}

void label_exit::mousePressEvent(QMouseEvent* ev)
{
    emit Mouse_Pressed_Exit();
}
