#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QScrollArea>

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

private:
    void setupUi();

    ScanManager *m_scanManager;

    QListWidget *m_thumnailList;
    QLabel *m_previewLabel;
    QScrollArea *m_scrollArea;
};
