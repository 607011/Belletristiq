/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#include <random>

#include <QtConcurrent>
#include <QFuture>
#include <QSettings>
#include <QString>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>
#include <QMimeData>
#include <QElapsedTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "markovnode.h"
#include "markovnode.h"
#include "markovchain.h"


class MainWindowPrivate {
public:
  MainWindowPrivate(void)
    : markovChain(new MarkovChain)
    , pDist(0.0, 1.0)
    , textFilesLoaded(0)
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
  QFuture<void> loadTextFuture;
  int textFilesLoaded;
  QElapsedTimer stopwatch;
};


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , d_ptr(new MainWindowPrivate)
{
  ui->setupUi(this);

  ui->tokensProgressBar->hide();
  ui->filesProgressBar->hide();

  QObject::connect(this, SIGNAL(loadingTextFile(QString)), this, SLOT(onTextFilesLoading(QString)));
  QObject::connect(this, SIGNAL(textFilesLoadFinished()), this, SLOT(onTextFilesLoaded()));
  QObject::connect(d_ptr->markovChain, SIGNAL(progressValueChanged(int)), ui->tokensProgressBar, SLOT(setValue(int)));
  QObject::connect(d_ptr->markovChain, SIGNAL(progressRangeChanged(int, int)), ui->tokensProgressBar, SLOT(setRange(int,int)));

  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionLoadTextFiles, SIGNAL(triggered(bool)), SLOT(onLoadTextFiles()));
  QObject::connect(ui->actionSaveMarkovChain, SIGNAL(triggered(bool)), SLOT(onSaveMarkovChain()));
  QObject::connect(ui->actionLoadMarkovChain, SIGNAL(triggered(bool)), SLOT(onLoadMarkovChain()));
  QObject::connect(ui->actionResetMarkovChain, SIGNAL(triggered(bool)), SLOT(onResetMarkovChain()));
  QObject::connect(ui->generatePushButton, SIGNAL(clicked(bool)), SLOT(onGenerateText()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));

  restoreSettings();

  setUnifiedTitleAndToolBarOnMac(true);
  setAcceptDrops(true);
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  Q_D(MainWindow);
  if (d->loadTextFuture.isRunning()) {
    d->markovChain->cancel();
    d->loadTextFuture.waitForFinished();
  }
  saveSettings();
  e->accept();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
  if (e->mimeData()->hasUrls()) {
    e->acceptProposedAction();
  }
}


void MainWindow::dragMoveEvent(QDragMoveEvent *)
{
  // do nothing
}


void MainWindow::dragLeaveEvent(QDragLeaveEvent *)
{
  // do nothing
}


void MainWindow::dropEvent(QDropEvent *e)
{
  if (e->mimeData()->hasUrls()) {
    QStringList textFileNames;
    foreach (QUrl url, e->mimeData()->urls()) {
      const QString &fileName = url.toLocalFile();
      if (fileName.endsWith(".txt")) {
        textFileNames << fileName;
      }
    }
    loadTextFiles(textFileNames);
    e->acceptProposedAction();
  }
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


QString MainWindow::generateText_Simple(void)
{
  Q_D(MainWindow);
  std::uniform_int_distribution<int> nDist(0, d->markovChain->count() - 1);
  MarkovNode *node = Q_NULLPTR;
  QString lastToken;
  QString result;
  int N = ui->wordCountSpinBox->value();
  while (N-- > 0) {
    if (node == Q_NULLPTR) {
      node = d->markovChain->at(nDist(d->rng));
      int nTries = d->markovChain->count() / 2;
      do {
        node = d->markovChain->at(nDist(d->rng));
      } while (!node->token().at(0).isUpper() && nTries-- > 0);
      if (!result.isEmpty()) {
        result += " \\\n";
      }
    }
    static const QStringList StopTokens = { ".", ",", ":", ";", "?", "!", ")", "«", "_" };
    const QString &token = node->token();
    if (!lastToken.isEmpty() && !StopTokens.contains(token)) {
      result += " ";
    }
    result += token;
    lastToken = token;
    node = node->selectSuccessor(d->pDist(d->rng));
  }
  return result;
}


void MainWindow::onGenerateText(void)
{
  QString generatedText;
  if (ui->algorithmComboBox->currentText() == tr("Simple")) {
    generatedText = generateText_Simple();
  }
  ui->plainTextEdit->setPlainText(generatedText);
}


void MainWindow::onTextFilesLoadCanceled(void)
{
  ui->statusbar->showMessage(tr("Cancelled."), 3000);
  ui->tokensProgressBar->hide();
  ui->filesProgressBar->hide();
  setCursor(Qt::ArrowCursor);
}


void MainWindow::onTextFilesLoaded(void)
{
  Q_D(MainWindow);
  ui->statusbar->showMessage(tr("Files loaded in %1 seconds.").arg(d->stopwatch.elapsed() / 1000), 3000);
  ui->tokensProgressBar->hide();
  ui->filesProgressBar->hide();
  setCursor(Qt::ArrowCursor);
  ui->generatePushButton->setEnabled(true);
  ui->plainTextEdit->setEnabled(true);
  onGenerateText();
}


void MainWindow::onTextFilesLoading(const QString &filename)
{
  Q_D(MainWindow);
  ++d->textFilesLoaded;
  ui->filesProgressBar->setValue(d->textFilesLoaded);
  ui->statusbar->showMessage(tr("Loading %1 ...").arg(filename));
}


void MainWindow::loadTextFilesThread(const QStringList &textFileNames)
{
  Q_D(MainWindow);
  foreach (QString textFilename, textFileNames) {
    if (!d->markovChain->isCancelled()) {
      emit loadingTextFile(QFileInfo(textFilename).fileName());
      d->markovChain->readFromTextFile(textFilename);
    }
  }
  d->markovChain->postProcess();
  emit textFilesLoadFinished();
}


void MainWindow::loadTextFiles(QStringList textFilenames)
{
  Q_D(MainWindow);
  if (textFilenames.count() > 0) {
    d->stopwatch.start();
    d->textFilesLoaded = 0;
    ui->filesProgressBar->setRange(0, textFilenames.count());
    ui->filesProgressBar->setValue(0);
    setCursor(Qt::WaitCursor);
    ui->generatePushButton->setEnabled(false);
    ui->plainTextEdit->setEnabled(false);
    d->lastLoadTextDirectory = QFileInfo(textFilenames.first()).absolutePath();
    ui->tokensProgressBar->show();
    ui->filesProgressBar->show();
    d->loadTextFuture = QtConcurrent::run(this, &MainWindow::loadTextFilesThread, textFilenames);
  }
}


void MainWindow::onLoadTextFiles(void)
{
  Q_D(MainWindow);
  QStringList textFilenames = QFileDialog::getOpenFileNames(
        this,
        tr("Load text files ..."),
        d->lastLoadTextDirectory,
        tr("Text files (*.txt)"));
  loadTextFiles(textFilenames);
}


void MainWindow::onSaveMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getSaveFileName(
        this,
        tr("Save Markov chain to ..."),
        d->lastSaveMarkovDirectory,
        tr("Markov files (*.markov *.markovz *.json .jsonz)"));
  if (!markovFilename.isEmpty()) {
    d->lastSaveMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    d->markovChain->save(markovFilename);
  }
}


void MainWindow::onLoadMarkovChain(void)
{
  Q_D(MainWindow);
  QString markovFilename = QFileDialog::getOpenFileName(
        this,
        tr("Load Markov chain from ..."),
        d->lastLoadMarkovDirectory,
        tr("Markov files (*.markov *.markovz *.json .jsonz)"));
  if (!markovFilename.isEmpty()) {
    d->lastLoadMarkovDirectory = QFileInfo(markovFilename).absolutePath();
    d->markovChain->readFromJsonFile(markovFilename);
    onGenerateText();
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


void MainWindow::about(void)
{
  QMessageBox::about(
        this, tr("About %1 %2").arg(AppName).arg(AppVersion),
        tr("<p><b>%1</b> is a Markov-chain based text generator. "
           "See <a href=\"%2\" title=\"%1 project homepage\">%2</a> for more info.</p>"
           "<p>This program is free software: you can redistribute it and/or modify "
           "it under the terms of the GNU General Public License as published by "
           "the Free Software Foundation, either version 3 of the License, or "
           "(at your option) any later version.</p>"
           "<p>This program is distributed in the hope that it will be useful, "
           "but WITHOUT ANY WARRANTY; without even the implied warranty of "
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
           "GNU General Public License for more details.</p>"
           "You should have received a copy of the GNU General Public License "
           "along with this program. "
           "If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0\">http://www.gnu.org/licenses</a>.</p>"
           "<p>Copyright &copy; 2015 %3 &lt;%4&gt;.</p>"
           )
        .arg(AppName).arg(AppURL).arg(AppAuthor).arg(AppAuthorMail));
}


void MainWindow::aboutQt(void)
{
  QMessageBox::aboutQt(this);
}
