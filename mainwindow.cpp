/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#include <random>

#include <QString>
#include <QDateTime>
#include <QFileDialog>
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
};


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , d_ptr(new MainWindowPrivate)
{
  Q_D(MainWindow);
  ui->setupUi(this);

  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionSaveMarkovChain, SIGNAL(triggered(bool)), SLOT(onSaveMarkovChain()));
  QObject::connect(ui->actionLoadMarkovChain, SIGNAL(triggered(bool)), SLOT(onLoadMarkovChain()));

  d->markovChain->readFromFile("/Users/olau/Workspace/Belletristiq/Heine/1.txt");
  d->markovChain->readFromFile("/Users/olau/Workspace/Belletristiq/Heine/2.txt");
  d->markovChain->readFromFile("/Users/olau/Workspace/Belletristiq/Heine/3.txt");
  d->markovChain->readFromFile("/Users/olau/Workspace/Belletristiq/Heine/4.txt");
  d->markovChain->postProcess();

  std::uniform_int_distribution<int> nDist(0, d->markovChain->count() - 1);
  MarkovNode *node = Q_NULLPTR;
  int N = 400;
  QString lastToken;
  QString result;
  while (--N > 0) {
    if (node == Q_NULLPTR) {
      node = d->markovChain->at(nDist(d->rng));
    }
    const QString &token = node->token();
    if (!lastToken.isEmpty() && token != "." && token != "," && token != ":" && token != ";" && token != "?" && token != "!" && token != ")") {
      result += " ";
    }
    result += token;
    node = node->selectSuccessor(d->pDist(d->rng));
    lastToken = token;
  }
  ui->plainTextEdit->setPlainText(result);
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::onSaveMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getSaveFileName(this, tr("Save Markov chain to ..."));
  if (!markovFilename.isEmpty()) {
    QFile outFile(markovFilename);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      QJsonDocument json = QJsonDocument::fromVariant(d->markovChain->toVariantList());
      outFile.write(json.toJson());
      outFile.close();
    }
  }
}


void MainWindow::onLoadMarkovChain(void)
{

}
