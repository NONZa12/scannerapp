#include <QDebug>
#include <QImage>
#include <QtWin>
#include <QPdfWriter>
#include <QPageSize>
#include <QPainter>

#include "ScanManager.h"

ScanManager::ScanManager(QObject *parent) : QObject(parent)
{
    if (!DTWAIN_SysInitialize())
    {
        qDebug() << "Failed to initialize DTWAIN";
    }
}

ScanManager::~ScanManager()
{
    DTWAIN_SysDestroy();
}

bool ScanManager::initScanner()
{
    m_SelectedSource = DTWAIN_SelectSource();
    return (m_SelectedSource != nullptr);
}

void ScanManager::startScanning()
{
    if (!m_SelectedSource) {
        if(!initScanner()) return;
    }

    DTWAIN_ARRAY images = DTWAIN_AcquireNative(
        m_SelectedSource, DTWAIN_PT_DEFAULT, DTWAIN_MAXACQUIRE,true, true, NULL);

    if (images)
    {
        LONG count = DTWAIN_ArrayGetCount(images);
        for (int i=0; i<count; ++i)
        {
            HANDLE hBitmap = DTWAIN_GetAcquiredImage(images, i, 0);
            if (hBitmap)
            {
                QImage img = QtWin::fromHBITMAP((HBITMAP)hBitmap).toImage();
                if (!img.isNull())
                {
                    emit imageAcquired(img);
                }
            }
        }
        DTWAIN_ArrayDestroy(images);
    }
}

bool ScanManager::saveToPdf(const QString &filePath, const QList<QImage> &images)
{
    if (images.isEmpty()) return false;

    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter painter(&writer);
    QRect writerRect = writer.pageLayout().paintRectPixels(writer.resolution());

    for (int i=0; i<images.size(); ++i)
    {
        if (i>0) writer.newPage();

        QImage scaledImg = images[i].scaled(
            writerRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (writerRect.width() - scaledImg.width()) / 2;
        int y = (writerRect.height() - scaledImg.height()) / 2;

        painter.drawImage(x, y, scaledImg);
    }
    painter.end();
    
    return true;
}