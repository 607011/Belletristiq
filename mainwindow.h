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
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace Ui {
class MainWindow;
}

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  enum ParseMethod { Words, Tuples };

  explicit MainWindow(QWidget *parent = Q_NULLPTR);
  ~MainWindow();

signals:
  void textFilesLoadFinished(void);
  void loadingTextFile(QString);
  void postProcessingTextFiles(void);

protected:
  void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
  void dragEnterEvent(QDragEnterEvent *) Q_DECL_OVERRIDE;
  void dragMoveEvent(QDragMoveEvent *) Q_DECL_OVERRIDE;
  void dragLeaveEvent(QDragLeaveEvent *) Q_DECL_OVERRIDE;
  void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

private slots:
  void onMethodChanged(QAction*);
  void onLoadTextFiles(void);
  void onSaveMarkovChain(void);
  void onLoadMarkovChain(void);
  void onResetMarkovChain(void);
  void onTextFilesLoadCanceled(void);
  void onTextFilesLoaded(void);
  void onTextFilesLoading(const QString &);
  void onGenerateText(void);
  void about(void);
  void aboutQt(void);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

private:
  void saveSettings(void);
  void restoreSettings(void);
  void loadTextFilesThread(const QStringList &textFileNames);
  void loadTextFiles(QStringList textFilenames);
  QString generateText_FromWords(void);
  QString generateText_FromTuples(void);
  void parseText(const QString &text, QStringList &tokens, int &totalSize);
};

#endif // __MAINWINDOW_H_
