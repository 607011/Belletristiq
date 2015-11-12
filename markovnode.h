/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#ifndef __MARKOVNODE_H_
#define __MARKOVNODE_H_

#include <QList>
#include <QString>
#include <QVariantMap>
#include <QVariantList>


class MarkovChain;
class MarkovEdge;


class MarkovNode {
public:
  typedef QList<MarkovEdge*> MarkovEdgeList;

  explicit MarkovNode(const QString &mToken, int id);

  void addSuccessor(MarkovNode *node);
  void calcProbabilities(void);

  const MarkovEdgeList &successors(void) const;
  void setSuccessors(const MarkovEdgeList &successors);
  const QVariantList &preliminarySuccessors(void) const;
  void setPreliminarySuccessors(const QVariantList &successors);
  const QString &token(void) const;
  int id(void) const;

  MarkovNode *selectSuccessor(qreal p);

  void postProcess(const MarkovChain *chain);

  QVariantMap toVariantMap(void) const;

  static MarkovNode *fromVariantMap(const QVariantMap &map);

private:
  QString mToken;
  int mId;
  MarkovEdgeList mSuccessors;
  QVariantList mPreliminarySuccessors;
};


QDebug operator<<(QDebug debug, const MarkovNode &node);



#endif // __MARKOVNODE_H_

