#ifndef LABEL_MINIMIZE_H
#define LABEL_MINIMIZE_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

class label_minimize : public QLabel
{
    Q_OBJECT
public:
    label_minimize(QWidget* parent = 0);

    void mousePressEvent(QMouseEvent* ev);

signals:
    void Mouse_Pressed_Minimize();

public slots:

};

#endif // LABEL_MINIMIZE_H
