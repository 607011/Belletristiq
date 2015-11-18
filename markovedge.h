/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#ifndef __MARKOVEDGE_H_
#define __MARKOVEDGE_H_

#include <QtGlobal>
#include <QString>

class MarkovNode;

class MarkovEdge {
public:
  explicit MarkovEdge(MarkovNode *);
  MarkovEdge(MarkovNode *, int count);

  MarkovNode *node(void);
  int count(void) const;
  void setCount(int);
  qreal p(void) const;
  void setProbability(qreal p);
  void increaseCount(void);

  QString toString(void) const;

private:
  MarkovNode *mNode;
  int mCount;
  qreal mProbability;
};


#endif // __MARKOVEDGE_H_

