#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QScrollArea>
#include <QMap>
#include <QComboBox>
#include <QCheckBox>

#include "ScanManager/ScanManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onScanClicked();
    void onSavePdfClicked();
    void onImageReceived(QImage img);
    void onThumnailClicked(QListWidgetItem *item);
    void updatePageNumbers();

private:
    void setupUi();

    ScanManager *m_scanManager;

    QListWidget *m_thumnailList;
    QLabel *m_previewLabel;
    QScrollArea *m_scrollArea;
    QComboBox *m_comboResolution;
    QComboBox *m_comboColorMode;
    QCheckBox *m_checkDuplex;

    QMap<int, QImage> m_imageMap;
    int m_nextId = 0;
};
