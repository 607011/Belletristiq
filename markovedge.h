/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#ifndef __MARKOVEDGE_H_
#define __MARKOVEDGE_H_

#include <QtGlobal>

class MarkovNode;

class MarkovEdge {
public:
  explicit MarkovEdge(MarkovNode *mNode);

  MarkovNode *node(void);
  int count(void) const;
  qreal p(void) const;
  void setProbability(qreal p);
  void increaseCount(void);

private:
  MarkovNode *mNode;
  int mCount;
  qreal mProbability;
};


#endif // __MARKOVEDGE_H_

