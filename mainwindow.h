/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>
#include <QScopedPointer>
#include <QCloseEvent>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = Q_NULLPTR);
  ~MainWindow();

signals:
  void textFilesLoadFinished(void);
  void loadingTextFile(QString);

protected:
  void closeEvent(QCloseEvent *);

private slots:
  void onLoadTextFiles(void);
  void onSaveMarkovChain(void);
  void onLoadMarkovChain(void);
  void onResetMarkovChain(void);
  void generateText_Simple(void);
  void onTextFilesLoadCanceled(void);
  void onTextFilesLoaded(void);
  void onTextFilesLoading(const QString &);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

private:
  void saveSettings(void);
  void restoreSettings(void);
  void loadTextFilesThread(const QStringList &textFileNames);

};

#endif // __MAINWINDOW_H_
