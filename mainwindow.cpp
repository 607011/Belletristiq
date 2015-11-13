/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#include <random>

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
  ~MainWindowPrivate()
  {
    if (markovChain != Q_NULLPTR) {
      delete markovChain;
    }
  }

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

  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionLoadTextFiles, SIGNAL(triggered(bool)), SLOT(onLoadTextFiles()));
  QObject::connect(ui->actionSaveMarkovChain, SIGNAL(triggered(bool)), SLOT(onSaveMarkovChain()));
  QObject::connect(ui->actionLoadMarkovChain, SIGNAL(triggered(bool)), SLOT(onLoadMarkovChain()));
  QObject::connect(ui->actionResetMarkovChain, SIGNAL(triggered(bool)), SLOT(onResetMarkovChain()));
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
      node = d->markovChain->at(nDist(d->rng)); // XXX
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


void MainWindow::onLoadTextFiles(void)
{
  Q_D(MainWindow);
  QStringList textFilenames = QFileDialog::getOpenFileNames(
        this,
        tr("Load text files ..."),
        d->lastLoadTextDirectory,
        tr("Text files (*.txt)"));
  if (!textFilenames.isEmpty()) {
    foreach (QString textFilename, textFilenames) {
      d->lastLoadTextDirectory = QFileInfo(textFilename).absolutePath();
      d->markovChain->readFromTextFile(textFilename);
    }
    d->markovChain->postProcess();
  }
  generateText();
}


void MainWindow::onSaveMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getSaveFileName(
        this,
        tr("Save Markov chain to ..."),
        d->lastSaveMarkovDirectory,
        tr("Markov files (*.json *.dat *.markov)"));
  if (!markovFilename.isEmpty()) {
    d->lastSaveMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    d->markovChain->save(markovFilename, false);
  }
}


void MainWindow::onLoadMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getOpenFileName(
        this,
        tr("Load Markov chain from ..."),
        d->lastLoadMarkovDirectory,
        tr("Markov files (*.json *.dat *.markov)"));
  if (!markovFilename.isEmpty()) {
    d->lastLoadMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    d->markovChain->readFromJsonFile(markovFilename);
    generateText();
  }
}


void MainWindow::onResetMarkovChain(void)
{
  Q_D(MainWindow);
  // TODO: QMessageBox::question() should ask user if she really wants to reset the Markov chain
  d->markovChain->clear();
  ui->plainTextEdit->clear();
  ui->statusbar->showMessage(tr("Markov chain reset."), 3000);
}
