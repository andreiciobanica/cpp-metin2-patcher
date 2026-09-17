#ifndef LABEL_EXIT_H
#define LABEL_EXIT_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

class label_exit : public QLabel
{
    Q_OBJECT
public:
    label_exit(QWidget* parent = 0);

    void mousePressEvent(QMouseEvent* ev);

signals:
    void Mouse_Pressed_Exit();

public slots:

};

#endif // LABEL_EXIT_H
