#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPalette>
#include <QMessageBox>
#include <QFileDialog>

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

    //Layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    //Thumnail list
    m_thumnailList = new QListWidget();
    m_thumnailList->setFixedWidth(250);
    m_thumnailList->setViewMode(QListWidget::IconMode);
    m_thumnailList->setIconSize(QSize(180, 240));
    m_thumnailList->setSpacing(10);
    m_thumnailList->setResizeMode(QListWidget::Adjust);

    //make drag & drop
    m_thumnailList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_thumnailList->setDragEnabled(true);
    m_thumnailList->setAcceptDrops(true);
    m_thumnailList->setDropIndicatorShown(true);
    m_thumnailList->setDragDropMode(QAbstractItemView::InternalMove);

    connect(m_thumnailList, &QListWidget::itemClicked, this, &MainWindow::onThumnailClicked);

    //Preview area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setBackgroundRole(QPalette::Dark);

    m_previewLabel = new QLabel("Ready to Scan");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_previewLabel);

    //Add to Layout
    layout->addWidget(m_thumnailList);
    layout->addWidget(m_scrollArea);
}

void MainWindow::onScanClicked()
{
    m_previewLabel->setText("Scanning... Please wait.");
    m_scanManager->startScanning();
}

void MainWindow::onImageReceived(QImage img)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(QString("Page %1").arg(m_thumnailList->count() + 1));

    item->setIcon(QIcon(QPixmap::fromImage(img)));

    item->setData(Qt::UserRole, QVariant(img));

    m_thumnailList->addItem(item);

    m_previewLabel->setPixmap(QPixmap::fromImage(img));
    m_thumnailList->scrollToBottom();
}

void MainWindow::onThumnailClicked(QListWidgetItem *item)
{
    QImage img = item->data(Qt::UserRole).value<QImage>();
    m_previewLabel->setPixmap(QPixmap::fromImage(img));
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
        imagesToSave.append(item->data(Qt::UserRole).value<QImage>());
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
