/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#ifndef __MARKOVCHAIN_H_
#define __MARKOVCHAIN_H_

#include <QDebug>
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>

#include "markovnode.h"


class MarkovChain : public QObject {
  Q_OBJECT

public:
  typedef QMap<QString, MarkovNode*> MarkovNodeMap;

  MarkovChain(void);

  void add(const QStringList &tokenList);
  const MarkovNodeMap &nodes(void) const;
  void postProcess(void);
  void clear(void);
  bool isCancelled(void) const;
  void cancel(void);

  int count(void) const;
  MarkovNode *at(int);

  bool readFromTextFile(const QString &filename);
  bool readFromMarkovFile(const QString &filename);
  void save(const QString &filename);

  QString toString(void) const;

  static const QByteArray FileHeader;

  void addText(const QString &text);

signals:
  void progressRangeChanged(int, int);
  void progressValueChanged(int);

private:
  MarkovNodeMap mNodeMap;
  volatile bool mCancelled;

private:
  void parseText(const QString &line, QStringList &tokens, int &totalSize);
};


QDebug operator<<(QDebug debug, const MarkovChain &chain);



#endif // __MARKOVCHAIN_H_

