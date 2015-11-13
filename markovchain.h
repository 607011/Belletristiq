/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#ifndef __MARKOVCHAIN_H_
#define __MARKOVCHAIN_H_


#include <QDebug>
#include <QByteArray>
#include <QVariantMap>

#include "markovnode.h"


class MarkovChain {
public:
  typedef QMap<QString, MarkovNode*> MarkovNodeMap;

  MarkovChain(void);

  void add(const QStringList &tokenList);
  const MarkovNodeMap &nodes(void) const;
  void postProcess(void);
  void clear(void);

  int count(void) const;
  MarkovNode *at(int);

  bool readFromTextFile(const QString &filename);
  bool readFromJsonFile(const QString &filename);
  void save(const QString &filename, bool compressed);

  QVariantMap toVariantMap(void);

  static const QByteArray FileHeader;

private:
  MarkovNodeMap mNodeMap;
};


QDebug operator<<(QDebug debug, const MarkovChain &chain);



#endif // __MARKOVCHAIN_H_

