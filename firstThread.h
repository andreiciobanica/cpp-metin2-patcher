#ifndef FIRSTTHREAD_H
#define FIRSTTHREAD_H
#include <QThread>
#include <QtCore>
#include <QDebug>
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

class firstThread : public QThread
{
	Q_OBJECT

public:
	explicit firstThread(QObject *parent = 0);
	firstThread();
	void run();

signals:
	void afterFirstThread(QStringList);
	//void updateTotalProgressMaximumValue(int);

private:

public slots:
};

#endif