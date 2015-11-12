/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include <QApplication>
#include <QDebug>
#include "mainwindow.h"
#include "globals.h"


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationName(AppCompanyName);
  QCoreApplication::setApplicationName(AppName);
  MainWindow w;
  w.show();
  return a.exec();
}
