#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "label_exit.h"
#include "label_minimize.h"
#include "downloadManager.h"
#include "DownloadFile.h"
#include "config.h"
#include <QMouseEvent>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setApplicationName(PatcherConfig::WindowTitle);
    ui->setupUi(this);
    this->setWindowTitle(PatcherConfig::WindowTitle);
    this->setWindowIcon(QIcon(":/noprefix/app.ico"));
    this->setFixedSize(this->size());
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->progressBar->setValue(0);
    
    QPixmap bkgnd(":/noprefix/background.png");
    QPalette bkgndpalette;
    bkgndpalette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(bkgndpalette);

    ui->startGame->setStyleSheet("border-image:url(:/noprefix/start-disabled.png);");

    ui->startConfig->setStyleSheet("border-image:url(:/noprefix/setari.png);");

    connect(ui->lblminimized, SIGNAL(Mouse_Pressed_Minimize()), this, SLOT(Mouse_Pressed_Minimize()));
    connect(ui->lblexit, SIGNAL(Mouse_Pressed_Exit()), this, SLOT(Mouse_Pressed_Exit()));

    ui->startGame->setEnabled(false);

    patchStart();
}

void MainWindow::patchStart() {
    manager_config = new DownloadFile();
    connect(manager_config, SIGNAL(finished()), this, SLOT(cfgStart()));
    connect(manager_config, SIGNAL(updateProgressLabel(QString)), this, SLOT(updateProgressLabel(QString)));

    QStringList arguments;

    arguments.append(QString::fromStdString(std::string(PatcherConfig::ServerUrl) + "patchlist.txt"));
    arguments.append(QString::fromStdString(std::string(PatcherConfig::ServerUrl) + "folders.txt"));
    arguments.append(QString::fromStdString(std::string(PatcherConfig::ServerUrl) + "exceptions.txt"));

    manager_config->append(arguments);
}

void MainWindow::cfgStart() {
    ui->nameLabel->setText(QString::fromStdString("Preparing..."));
    cfgFiles = new firstThread(this);
    cfgFiles->start();

    connect(cfgFiles, SIGNAL(afterFirstThread(QStringList)), this, SLOT(onAction(QStringList)));
}

void MainWindow::onAction(QStringList arguments) {

    if (!arguments.isEmpty()) {
        manager = new DownloadManager();

        manager->append(arguments);

        connect(manager, SIGNAL(updateProgress(qint64, qint64, QString)), this, SLOT(updateProgressStatus(qint64, qint64, QString)));
        connect(manager, SIGNAL(updateProgressLabel(QString)), this, SLOT(updateProgressLabel(QString)));
        connect(manager, SIGNAL(activateStartButton()), this, SLOT(onactivateStartButton()));
    }
    else {
        onactivateStartButton();
        int val = 100;
        ui->progressBar->setMaximum(val);
        ui->progressBar->setValue(val);
    }
}

void MainWindow::on_startConfig_clicked() {
    QProcess::startDetached(PatcherConfig::ConfigExecutable);
}

void MainWindow::on_startGame_clicked() {
    QProcess::startDetached(PatcherConfig::GameExecutable);
    QApplication::quit();
}

void MainWindow::updateProgressLabel(QString s) {
    ui->nameLabel->setText(s);
}

void MainWindow::onactivateStartButton() {
    ui->startGame->setStyleSheet("border-image:url(:/noprefix/start.png);");
    ui->startGame->setEnabled(true);
}

void MainWindow::updateProgressStatus(qint64 bytesReceived, qint64 bytesTotal, QString speed)
{
    ui->progressBar->setMaximum(bytesTotal);
    ui->progressBar->setValue(bytesReceived);
    ui->speedLabel->setText(speed);
}

void MainWindow::Mouse_Pressed_Exit() {
    if (cfgFiles->isRunning()) {
        cfgFiles->terminate();
    }
    QCoreApplication::quit();
}

void MainWindow::Mouse_Pressed_Minimize() {
    this->showMinimized();
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    m_nMouseClick_X_Coordinate = event->x();
    m_nMouseClick_Y_Coordinate = event->y();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    move(event->globalX() - m_nMouseClick_X_Coordinate, event->globalY() - m_nMouseClick_Y_Coordinate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

