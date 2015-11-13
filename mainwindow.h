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

protected:
  void closeEvent(QCloseEvent *);

private slots:
  void onLoadTextFiles(void);
  void onSaveMarkovChain(void);
  void onLoadMarkovChain(void);
  void onResetMarkovChain(void);
  void generateText(void);

private:
  void saveSettings(void);
  void restoreSettings(void);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)
};

#endif // __MAINWINDOW_H_
