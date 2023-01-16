// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mainwindow.h"

#include <DOcr>

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtDebug>

static constexpr int ImageMaxWidth = 640;
static constexpr int ImageMaxHeight = 480;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , imageLabel(new QLabel)
    , pathEdit(new QLineEdit)
    , fileDialogButton(new QPushButton(tr("...")))
    , openImageButton(new QPushButton(tr("Open")))
    , languageBox(new QComboBox)
    , preButton(new QPushButton(tr("Previous box")))
    , nextButton(new QPushButton(tr("Next box")))
    , showCharRectButton(new QPushButton(tr("Show charracter anchor box")))
    , rectTextEdit(new QTextEdit)
    , simpleTextEdit(new QTextEdit)
    , ocrEngine(new Dtk::Ocr::DOcr)
{
    initOcrEngine();
    initUI();
    initConnect();
}

void MainWindow::runOcrAnalyze(const QString &path)
{
    QImage image(path);
    int width = image.width();
    int height = image.height();
    if (width > height || width == height) {
        if (width > ImageMaxWidth) {
            image = image.scaledToWidth(ImageMaxWidth, Qt::SmoothTransformation);
        }
    } else {  // height > width
        if (height > ImageMaxHeight) {
            image = image.scaledToHeight(ImageMaxHeight, Qt::SmoothTransformation);
        }
    }

    ocrEngine->setImage(image);
    ocrEngine->setLanguage(languageBox->currentText());
    ocrEngine->analyze();
    simpleTextEdit->setText(ocrEngine->simpleResult());
    currentRectIndex = 0;
    pathCached = path;

    switchBox(currentRectIndex);
}

void MainWindow::flushLabel()
{
    auto image = ocrEngine->imageCached();
    auto textBoxes = ocrEngine->textBoxes();
    if (textBoxes.isEmpty()) {
        imageLabel->setPixmap(QPixmap::fromImage(image));
        return;
    }

    QPainter paint;
    paint.begin(&image);
    paint.setPen(QPen(QColor("#FF0000"), 1));

    auto drawFunc = [&paint](const Dtk::Ocr::TextBox &box) {
        auto points = box.points;
        auto pointVec = points.toVector();
        paint.drawPolygon(pointVec.data(), pointVec.size());
    };

    drawFunc(textBoxes[currentRectIndex]);

    if (needDrawCharRect) {
        auto charBoxes = ocrEngine->charBoxes(currentRectIndex);
        for (auto &box : charBoxes) {
            drawFunc(box);
        }
    }

    paint.end();

    imageLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::switchBox(int index)
{
    if (index >= 0 && index < ocrEngine->textBoxes().size()) {
        currentRectIndex = index;
        flushLabel();
        rectTextEdit->setText(ocrEngine->resultFromBox(currentRectIndex));
    } else {
        if (ocrEngine->textBoxes().isEmpty()) {
            imageLabel->setPixmap(QPixmap::fromImage(ocrEngine->imageCached()));
            rectTextEdit->clear();
        }
    }
}

void MainWindow::initOcrEngine()
{
    ocrEngine->loadDefaultPlugin();
    ocrEngine->setUseMaxThreadsCount(2);
    ocrEngine->setUseHardware({{Dtk::Ocr::GPU_Vulkan, 0}});
}

void MainWindow::initConnect()
{
    connect(fileDialogButton, &QPushButton::clicked, [this]() {
        auto filePath = QFileDialog::getOpenFileName(nullptr, tr("Choose a image file"), "", "Image (*.png *.jpg *.jpeg *.bmp)");
        if (!filePath.isEmpty()) {
            pathEdit->setText(filePath);
        }
    });

    connect(openImageButton, &QPushButton::clicked, [this]() {
        auto filePath = pathEdit->text();
        if (!filePath.isEmpty()) {
            runOcrAnalyze(filePath);
        }
    });

    connect(preButton, &QPushButton::clicked, [this]() { switchBox(currentRectIndex - 1); });

    connect(nextButton, &QPushButton::clicked, [this]() { switchBox(currentRectIndex + 1); });

    connect(showCharRectButton, &QPushButton::clicked, [this]() {
        if (needDrawCharRect) {
            showCharRectButton->setText(tr("Show charracter anchor box"));
            needDrawCharRect = false;
        } else {
            showCharRectButton->setText(tr("Hide charracter anchor box"));
            needDrawCharRect = true;
        }
        flushLabel();
    });

    connect(languageBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        Q_UNUSED(text)
        ocrEngine->breakAnalyze();
        runOcrAnalyze(pathCached);
    });
}

void MainWindow::initUI()
{
    imageLabel->setMinimumSize(ImageMaxWidth, ImageMaxHeight);
    languageBox->addItems(ocrEngine->languageSupport());

    auto fileChooseLayer = new QHBoxLayout;
    fileChooseLayer->addWidget(new QLabel(tr("Image File Path:")));
    fileChooseLayer->addWidget(pathEdit);
    fileChooseLayer->addWidget(fileDialogButton);
    fileChooseLayer->addWidget(openImageButton);

    auto rectControlLayer = new QHBoxLayout;
    rectControlLayer->addWidget(languageBox);
    rectControlLayer->addWidget(preButton);
    rectControlLayer->addWidget(nextButton);
    rectControlLayer->addWidget(showCharRectButton);

    auto textLayer = new QHBoxLayout;
    textLayer->addWidget(rectTextEdit);
    textLayer->addWidget(simpleTextEdit);

    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(imageLabel, 0, Qt::AlignCenter);
    allLayer->addLayout(fileChooseLayer);
    allLayer->addLayout(rectControlLayer);
    allLayer->addLayout(textLayer);

    auto center = new QWidget;
    center->setLayout(allLayer);

    setCentralWidget(center);
}
