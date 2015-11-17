/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#include <random>
#include <functional>

#include <QtConcurrent>
#include <QSettings>
#include <QString>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "markovnode.h"
#include "markovnode.h"
#include "markovchain.h"


class MainWindowPrivate {
public:
  MainWindowPrivate(void)
    : markovChain(new MarkovChain)
    , pDist(0.0, 1.0)
  {
    rng.seed(QDateTime::currentDateTimeUtc().toTime_t());
  }
  ~MainWindowPrivate() {}

  MarkovChain *markovChain;
  std::mt19937 rng;
  std::uniform_real_distribution<qreal> pDist;
  QSettings settings;
  QString lastSaveMarkovDirectory;
  QString lastLoadMarkovDirectory;
  QString lastLoadTextDirectory;
};


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , d_ptr(new MainWindowPrivate)
{
  ui->setupUi(this);

  ui->progressBar->hide();

  QObject::connect(this, SIGNAL(textFilesLoadFinished()), this, SLOT(onTextFilesLoaded()));
  QObject::connect(d_ptr->markovChain, SIGNAL(progressValueChanged(int)), ui->progressBar, SLOT(setValue(int)));
  QObject::connect(d_ptr->markovChain, SIGNAL(progressRangeChanged(int, int)), ui->progressBar, SLOT(setRange(int,int)));

  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionLoadTextFiles, SIGNAL(triggered(bool)), SLOT(onLoadTextFiles()));
  QObject::connect(ui->actionSaveMarkovChain, SIGNAL(triggered(bool)), SLOT(onSaveMarkovChain()));
  QObject::connect(ui->actionLoadMarkovChain, SIGNAL(triggered(bool)), SLOT(onLoadMarkovChain()));
  QObject::connect(ui->generatePushButton, SIGNAL(clicked(bool)), SLOT(generateText()));

  restoreSettings();
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  saveSettings();
  e->accept();
}


void MainWindow::saveSettings(void)
{
  Q_D(MainWindow);
  d->settings.setValue("mainwindow/geometry", saveGeometry());
  d->settings.setValue("mainwindow/state", saveState());
  d->settings.setValue("options/lastSaveMarkovDirectory", d->lastSaveMarkovDirectory);
  d->settings.setValue("options/lastLoadMarkovDirectory", d->lastLoadMarkovDirectory);
  d->settings.setValue("options/lastLoadTextDirectory", d->lastLoadTextDirectory);
  d->settings.setValue("options/wordCount", ui->wordCountSpinBox->value());
  d->settings.sync();
}


void MainWindow::restoreSettings(void)
{
  Q_D(MainWindow);
  restoreGeometry(d->settings.value("mainwindow/geometry").toByteArray());
  restoreState(d->settings.value("mainwindow/state").toByteArray());
  d->lastSaveMarkovDirectory = d->settings.value("options/lastSaveMarkovDirectory").toString();
  d->lastLoadMarkovDirectory = d->settings.value("options/lastLoadMarkovDirectory").toString();
  d->lastLoadTextDirectory = d->settings.value("options/lastLoadTextDirectory").toString();
  ui->wordCountSpinBox->setValue(d->settings.value("options/wordCount", 500).toInt());
}


void MainWindow::generateText(void)
{
  Q_D(MainWindow);
  std::uniform_int_distribution<int> nDist(0, d->markovChain->count() - 1);
  MarkovNode *node = Q_NULLPTR;
  QString lastToken;
  QString result;
  int N = ui->wordCountSpinBox->value();
  while (N-- > 0) {
    if (node == Q_NULLPTR) {
      int nTries = d->markovChain->count() / 2;
      do {
        node = d->markovChain->at(nDist(d->rng));
      } while (!node->token().at(0).isUpper() && nTries-- > 0);
      if (!result.isEmpty()) {
        result += " \\\n";
      }
    }
    const QString &token = node->token();
    if (!lastToken.isEmpty() && token != "." && token != "," && token != ":" && token != ";" && token != "?" && token != "!" && token != ")") {
      result += " ";
    }
    result += token;
    lastToken = token;
    node = node->selectSuccessor(d->pDist(d->rng));
  }
  ui->plainTextEdit->setPlainText(result);
}


void MainWindow::onTextFilesLoadCanceled(void)
{
  ui->progressBar->hide();
}


void MainWindow::onTextFilesLoaded(void)
{
  ui->statusbar->showMessage(tr("Loaded."), 3000);
  ui->progressBar->hide();
  ui->generatePushButton->setEnabled(true);
  setCursor(Qt::ArrowCursor);
  generateText();
}


void MainWindow::loadTextFilesThread(const QStringList &textFileNames)
{
  Q_D(MainWindow);
  foreach (QString textFilename, textFileNames) {
    qDebug() << "Reading" << textFilename << "...";
    d->markovChain->readFromTextFile(textFilename);
  }
  qDebug() << "Postprocessing ...";
  d->markovChain->postProcess();
  emit textFilesLoadFinished();
}


void MainWindow::onLoadTextFiles(void)
{
  Q_D(MainWindow);
  QStringList textFilenames = QFileDialog::getOpenFileNames(this, tr("Load text files ..."), d->lastLoadTextDirectory);
  if (!textFilenames.isEmpty()) {
    setCursor(Qt::WaitCursor);
    d->lastLoadTextDirectory = QFileInfo(textFilenames.first()).absolutePath();
    ui->progressBar->show();
    QtConcurrent::run(this, &MainWindow::loadTextFilesThread, textFilenames);
  }
}


void MainWindow::onSaveMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getSaveFileName(this, tr("Save Markov chain to ..."), d->lastSaveMarkovDirectory, tr("Markov files (*.json *.dat *.markov)"));
  if (!markovFilename.isEmpty()) {
    d->lastSaveMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    QFile outFile(markovFilename);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      QJsonDocument json = QJsonDocument::fromVariant(d->markovChain->toVariantList());
      outFile.write(json.toJson(QJsonDocument::Compact));
      outFile.close();
    }
  }
}


void MainWindow::onLoadMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getOpenFileName(this, tr("Load Markov chain from ..."), d->lastLoadMarkovDirectory, tr("Markov files (*.json *.dat *.markov)"));
  if (!markovFilename.isEmpty()) {
    d->lastLoadMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    d->markovChain->readFromJsonFile(markovFilename);
    d->markovChain->postProcess();
    generateText();
  }
}
