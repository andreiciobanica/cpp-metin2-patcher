#ifndef DOWNLOADFILE_H
#define DOWNLOADFILE_H

#include <QtNetwork>
#include <QtCore>

class DownloadFile : public QObject
{
    Q_OBJECT
public:
    explicit DownloadFile(QObject* parent = nullptr);

    void append(const QUrl& url);
    void append(const QStringList& urls);
    static QString saveFileName(const QUrl& url);

signals:
    void finished();
    void updateProgress(qint64, qint64, QString);
    void updateProgressLabel(QString);
    void activateStartButton();

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    bool isHttpRedirect() const;
    void reportRedirect();

    QNetworkAccessManager manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply* currentDownload = nullptr;
    QFile output;
    QElapsedTimer downloadTimer;

    int downloadedCount = 0;
    int totalCount = 0;
};

#endif