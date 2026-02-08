#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPalette>
#include <QMessageBox>
#include <QFileDialog>
#include <QFormLayout>

#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_scanManager = new ScanManager(this);

    connect(m_scanManager, &ScanManager::imageAcquired, this, &MainWindow::onImageReceived);

    setupUi();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUi()
{
    this->setWindowTitle("Simple Scanner");
    this->resize(1024, 768);

    //Tool bar
    QToolBar *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(32, 32));

    QAction *actScan = toolbar->addAction("Scan Document");
    QAction *actSave = toolbar->addAction("Save to PDF");

    connect(actScan, &QAction::triggered, this, &MainWindow::onScanClicked);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSavePdfClicked);

    //Main Layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    //Resolution ComboBox
    m_comboResolution = new QComboBox();
    m_comboResolution->addItem("100 DPI", 100);
    m_comboResolution->addItem("200 DPI", 200);
    m_comboResolution->addItem("300 DPI", 300);
    m_comboResolution->setCurrentIndex(2); //Select 300 by default

    //Color Mode ComboBox
    m_comboColorMode = new QComboBox();
    m_comboColorMode->addItem("Black, White", 0);
    m_comboColorMode->addItem("Grayscale", 1);
    m_comboColorMode->addItem("Color", 2);
    m_comboColorMode->setCurrentIndex(2); //Select Color by default

    //Duplex CheckBox
    m_checkDuplex = new QCheckBox("Scan Both Sides(Duplex)");

    //Add to form layout
    QFormLayout *settingsLayout = new QFormLayout();
    settingsLayout->addRow(new QLabel("Resolution"), m_comboResolution);
    settingsLayout->addRow(new QLabel("Color Mode"), m_comboColorMode);
    settingsLayout->addRow(m_checkDuplex);

    //Thumnail list
    m_thumnailList = new QListWidget();
    m_thumnailList->setFixedWidth(250);
    m_thumnailList->setViewMode(QListWidget::ListMode);
    m_thumnailList->setIconSize(QSize(100, 140));
    m_thumnailList->setSpacing(5);

    //make drag & drop
    m_thumnailList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_thumnailList->setDragEnabled(true);
    m_thumnailList->setAcceptDrops(true);
    m_thumnailList->setDropIndicatorShown(true);
    m_thumnailList->setDragDropMode(QAbstractItemView::InternalMove);

    connect(m_thumnailList, &QListWidget::itemClicked, this, &MainWindow::onThumnailClicked);
    connect(m_thumnailList->model(), &QAbstractItemModel::rowsMoved, this, &MainWindow::updatePageNumbers);

    //Preview area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setBackgroundRole(QPalette::Dark);

    m_previewLabel = new QLabel("Ready to Scan");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_previewLabel);

    //Add layout and widget to mainLayout
    mainLayout->addLayout(settingsLayout);
    mainLayout->addWidget(m_thumnailList);
    mainLayout->addWidget(m_scrollArea);
}

void MainWindow::onScanClicked()
{
    
    m_previewLabel->setText("Scanning... Please wait.");

    int dpi = m_comboResolution->currentData().toInt();
    int pixelType = m_comboColorMode->currentData().toInt();
    int duplex = m_checkDuplex->isChecked();

    m_scanManager->startScanning(dpi, pixelType, duplex);
}

void MainWindow::onImageReceived(QImage img)
{
    int myId = m_nextId++;
    m_imageMap.insert(myId, img);

    QListWidgetItem *item = new QListWidgetItem();
    item->setText(QString("Page %1").arg(m_thumnailList->count() + 1));

    item->setIcon(QIcon(QPixmap::fromImage(img).scaled(100, 140, Qt::KeepAspectRatio)));

    item->setData(Qt::UserRole, myId);

    m_thumnailList->addItem(item);

    updatePageNumbers();

    m_previewLabel->setPixmap(QPixmap::fromImage(img));
    m_thumnailList->scrollToBottom();
}

void MainWindow::onThumnailClicked(QListWidgetItem *item)
{
    int id = item->data(Qt::UserRole).toInt();

    if (m_imageMap.contains(id));
    {
        QImage img = m_imageMap.value(id);
        m_previewLabel->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::onSavePdfClicked()
{
    if (m_thumnailList->count() == 0)
    {
        QMessageBox::warning(this, "No Images", "Please scan ddocuments first.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Save PDF", "", "PDF Files (*.pdf)");
    if (filename.isEmpty()) return;

    QList<QImage> imagesToSave;
    for (int i=0; i<m_thumnailList->count(); ++i)
    {
        QListWidgetItem *item = m_thumnailList->item(i);

        int id = item->data(Qt::UserRole).toInt();
        if (m_imageMap.contains(id))
        {
            imagesToSave.append(m_imageMap.value(id));
        }
    }

    if (m_scanManager->saveToPdf(filename, imagesToSave))
    {
        QMessageBox::information(this, "Success", "SAved successfully");
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to save PDF.");
    }
}

void MainWindow::updatePageNumbers()
{
    for (int i=0; i<m_thumnailList->count(); ++i)
    {
        QListWidgetItem *item = m_thumnailList->item(i);
        item->setText(QString("Page %1").arg(i+1));
    }
}
