/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#ifndef __MARKOVNODE_H_
#define __MARKOVNODE_H_

#include <QList>
#include <QString>


class MarkovChain;
class MarkovEdge;


class MarkovNode {
public:
  typedef QList<MarkovEdge*> MarkovEdgeList;

  explicit MarkovNode(const QString &token);

  void addSuccessor(MarkovNode *node);
  void addSuccessor(MarkovEdge *edge);
  void calcProbabilities(void);

  const MarkovEdgeList &successors(void) const;
  const QString &token(void) const;

  MarkovNode *selectSuccessor(const qreal p);

  QString toString(void) const;

private:
  QString mToken;
  MarkovEdgeList mSuccessors;
};


QDebug operator<<(QDebug debug, const MarkovNode &node);



#endif // __MARKOVNODE_H_

