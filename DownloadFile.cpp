#include "DownloadFile.h"

#include <cstdio>

using namespace std;

DownloadFile::DownloadFile(QObject* parent) : QObject(parent)
{
}

void DownloadFile::append(const QStringList& urls)
{
    for (const QString& urlAsString : urls)
        append(QUrl::fromEncoded(urlAsString.toLocal8Bit()));

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, &DownloadFile::finished);
}

void DownloadFile::append(const QUrl& url)
{
    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, &DownloadFile::startNextDownload);

    downloadQueue.enqueue(url);
    ++totalCount;
}

QString DownloadFile::saveFileName(const QUrl& url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).filePath();
    basename = basename.right(basename.size() - 1);

    if (basename.isEmpty())
        basename = "download";

    return basename;
}

void DownloadFile::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        emit finished();
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
        this, &DownloadFile::downloadProgress);
    connect(currentDownload, &QNetworkReply::finished,
        this, &DownloadFile::downloadFinished);
    connect(currentDownload, &QNetworkReply::readyRead,
        this, &DownloadFile::downloadReadyRead);

    // prepare the output
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTimer.start();
}

void DownloadFile::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
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

void DownloadFile::downloadFinished()
{
    output.close();

    if (currentDownload->error()) {
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
        output.remove();
    }
    else {
        if (isHttpRedirect()) {
            reportRedirect();
            output.remove();
        }
        else {
            printf("Succeeded.\n");
            ++downloadedCount;
        }
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void DownloadFile::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}

bool DownloadFile::isHttpRedirect() const
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
        || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadFile::reportRedirect()
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
