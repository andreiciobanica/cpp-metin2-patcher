#include "downloadmanager.h"

#include <cstdio>

using namespace std;

DownloadManager::DownloadManager(QObject* parent) : QObject(parent)
{
}

void DownloadManager::append(const QStringList& urls)
{
    for (const QString& urlAsString : urls)
        append(QUrl::fromEncoded(urlAsString.toLocal8Bit()));

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, &DownloadManager::finished);
}

void DownloadManager::append(const QUrl& url)
{
    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, &DownloadManager::startNextDownload);

    downloadQueue.enqueue(url);
    ++totalCount;
}

QString DownloadManager::saveFileName(const QUrl& url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).filePath();
    basename = basename.right(basename.size() - 1);

    if (basename.isEmpty())
        basename = "download"; 

    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        emit finished();
        emit updateProgressLabel("Finished");
        emit updateProgress(100, 100, "");
        emit activateStartButton();
        return;
    }

    QUrl url = downloadQueue.dequeue();
    QString filename = saveFileName(url);

    emit updateProgressLabel(filename);

    output.setFileName(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
            qPrintable(filename), url.toEncoded().constData(),
            qPrintable(output.errorString()));

        startNextDownload();
        return;
    }

    QNetworkRequest request(url);
    currentDownload = manager.get(request);
    connect(currentDownload, &QNetworkReply::downloadProgress,
        this, &DownloadManager::downloadProgress);
    connect(currentDownload, &QNetworkReply::finished,
        this, &DownloadManager::downloadFinished);
    connect(currentDownload, &QNetworkReply::readyRead,
        this, &DownloadManager::downloadReadyRead);

    // prepare the output
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTimer.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    double speed = bytesReceived * 1000.0 / downloadTimer.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    }
    else if (speed < 1024 * 1024) {
        speed /= 1024;
        unit = "kB/s";
    }
    else {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }

    QString sSpeed = QString::number(speed).left(QString::number(speed).size() - 2) + " " + unit;
    emit updateProgress(bytesReceived, bytesTotal, sSpeed);

}

void DownloadManager::downloadFinished()
{
    output.close();

    if (currentDownload->error()) {
        // download failed
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
        output.remove();
    }
    else {
        // let's check if it was actually a redirect
        if (isHttpRedirect()) {
            reportRedirect();
            output.remove();
        }
        else {
            printf("Succeeded.\n");
            ++downloadedCount;
            emit updateTotalProgress(downloadedCount);
        }
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}

bool DownloadManager::isHttpRedirect() const
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
        || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::reportRedirect()
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = currentDownload->request().url();
    QTextStream(stderr) << "Request: " << requestUrl.toDisplayString()
        << " was redirected with code: " << statusCode
        << '\n';

    QVariant target = currentDownload->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!target.isValid())
        return;
    QUrl redirectUrl = target.toUrl();
    if (redirectUrl.isRelative())
        redirectUrl = requestUrl.resolved(redirectUrl);
    QTextStream(stderr) << "Redirected to: " << redirectUrl.toDisplayString()
        << '\n';
}
