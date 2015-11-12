/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#ifndef __MARKOVNODE_H_
#define __MARKOVNODE_H_

#include <QList>
#include <QString>

class MarkovEdge;

class MarkovNode {
public:
  typedef QList<MarkovEdge*> MarkovEdgeList;

  explicit MarkovNode(const QString &mToken);

  void addSuccessor(MarkovNode *node);
  void calcProbabilities(void);

  const MarkovEdgeList &successors(void) const;
  const QString &token(void) const;

  MarkovNode *selectSuccessor(qreal p);

private:
  QString mToken;
  MarkovEdgeList mSuccessors;
};


QDebug operator<<(QDebug debug, const MarkovNode &node);



#endif // __MARKOVNODE_H_

