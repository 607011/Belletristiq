/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#ifndef __MARKOVCHAIN_H_
#define __MARKOVCHAIN_H_


#include <QDebug>

#include "markovnode.h"


class MarkovChain {
public:
  typedef QList<MarkovNode*> MarkovNodeList;

  MarkovChain(void);

  void add(const QStringList &tokenList);
  const MarkovNodeList &nodes(void) const;
  void postProcess(void);

  int count(void) const;
  MarkovNode *at(int);

private:
  bool find(const QString &token, MarkovNodeList::iterator &i);

  MarkovNodeList mNodeList;
};


QDebug operator<<(QDebug debug, const MarkovChain &chain);



#endif // __MARKOVCHAIN_H_

