#pragma once

#include <QObject>
#include <QImage>
#include <QList>
#include <QString>

#include "dtwain.h"

class ScanManager : public QObject
{
    Q_OBJECT
public:
    explicit ScanManager(QObject *parent = nullptr);
    ~ScanManager();

    bool initScanner();

    QImage DIBToQImage(HANDLE hDIB);
    void startScanning();

    bool saveToPdf(const QString &filePath, const QList<QImage> &images);

signals:
    void imageAcquired(QImage img);
    void errorOccurred(QString msg);

private:
    DTWAIN_SOURCE m_SelectedSource = nullptr;
};