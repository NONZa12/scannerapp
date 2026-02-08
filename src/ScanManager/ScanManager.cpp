#include <QDebug>
#include <QImage>
#include <QPdfWriter>
#include <QPageSize>
#include <QPainter>
#include <QBuffer>
#include <QImageReader>
#include <Windows.h>

#include "ScanManager.h"

ScanManager::ScanManager(QObject *parent) : QObject(parent)
{
    if (!DTWAIN_SysInitialize())
    {
        qDebug() << "Warning: Failed to initialize DTWAIN";
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

QImage ScanManager::DIBToQImage(HANDLE hDIB)
{
    if (!hDIB) return QImage();

    //Lock memory
    uchar *data = (uchar *)GlobalLock(hDIB);
    if (!data) return QImage();

    //Read Header
    BITMAPINFOHEADER *bmi = (BITMAPINFOHEADER *)data;

    //Calculate image size
    int nColors = 0;
    if (bmi->biBitCount <= 8)
    {
        nColors = (bmi->biClrUsed == 0) ? (1 << bmi->biBitCount) : bmi->biClrUsed;
    }
    DWORD imageSize = GlobalSize(hDIB);

    //Create BMP file header
    BITMAPFILEHEADER bmf;
    bmf.bfType = 0x4D42; // "BM" signature
    bmf.bfReserved1 = 0;
    bmf.bfReserved2 = 0;
    bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + bmi->biSize + (nColors * sizeof(RGBQUAD));
    bmf.bfSize = sizeof(BITMAPFILEHEADER) + imageSize;

    //Create QByteArray
    QByteArray packedBmp;
    packedBmp.append((const char *)&bmf, sizeof(BITMAPFILEHEADER));
    packedBmp.append((const char *)data, imageSize);

    //Unlock memory
    GlobalUnlock(hDIB);

    QImage result;
    result.loadFromData(packedBmp, "BMP");

    return result;
}

void ScanManager::startScanning(int dpi, int pixelType, bool duplex)
{
    if (!m_SelectedSource) {
        if(!initScanner()) return;
    }

    //set DPI
    if (!DTWAIN_SetResolution(m_SelectedSource, (DTWAIN_FLOAT)dpi))
    {
        qDebug() << "Warning: Failed to set resolution to" << dpi;
    }

    //set Pixel Type
    if (!DTWAIN_SetPixelType(m_SelectedSource, pixelType, DTWAIN_DEFAULT, TRUE))
    {
        qDebug() << "Warning: Failed to set pixel type";
    }

    //set Duplex
    DTWAIN_EnableDuplex(m_SelectedSource, duplex ? TRUE : FALSE);

    DTWAIN_ARRAY images = DTWAIN_AcquireNative(
        m_SelectedSource, pixelType, DTWAIN_MAXACQUIRE,true, true, NULL);

    if (images)
    {
        LONG count = DTWAIN_ArrayGetCount(images);
        for (int i=0; i<count; ++i)
        {
            HANDLE hBitmap = DTWAIN_GetAcquiredImage(images, i, 0);
            if (hBitmap)
            {
                QImage img = DIBToQImage(hBitmap);
                GlobalFree(hBitmap);
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