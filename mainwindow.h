#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "label_exit.h"
#include "label_minimize.h"
#include "downloadManager.h"
#include "DownloadFile.h"
#include "firstThread.h"
#include <QtCore>
#include <QProgressBar>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <map>
#include <openssl/sha.h>
#include <utility>

#pragma warning(disable : 4996)

using namespace std;
namespace fs = std::filesystem;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; };
class QSslError;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    firstThread* cfgFiles;

private slots:
    void updateProgressStatus(qint64, qint64, QString);
    void updateProgressLabel(QString);
    void onactivateStartButton();
    void Mouse_Pressed_Exit();
    void Mouse_Pressed_Minimize();
    void on_startConfig_clicked();
    void on_startGame_clicked();
    void onAction(QStringList);
    void patchStart();
    void cfgStart();

private:
    Ui::MainWindow* ui;
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    DownloadManager* manager;
    DownloadFile* manager_config;
};
#endif // MAINWINDOW_H
