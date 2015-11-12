/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#include "markovnode.h"
#include "markovedge.h"

#include <QtGlobal>
#include <QDebug>


MarkovNode::MarkovNode(const QString &token)
  : mToken(token)
{
  /* ... */
}


void MarkovNode::addSuccessor(MarkovNode *node)
{
  MarkovEdgeList::iterator i;
  for (i = mSuccessors.begin(); i != mSuccessors.end(); ++i) {
    if ((*i)->node() == node) {
      (*i)->increaseCount();
      break;
    }
  }
  if (i == mSuccessors.end()) {
    mSuccessors.append(new MarkovEdge(node));
  }
}


void MarkovNode::calcProbabilities(void) {
  int N = 0;
  foreach(MarkovEdge *edge, mSuccessors) {
    N += edge->count();
  }
  if (N > 0) {
    qreal p = 1.0;
    foreach(MarkovEdge *edge, mSuccessors) {
      p -= qreal(edge->count()) / N;
      if (qFuzzyIsNull(p)) {
        p = 0;
      }
      else {
        p = qMax(p, 0.0);
      }
      edge->setProbability(p);
    }
  }
}


const MarkovNode::MarkovEdgeList &MarkovNode::successors(void) const
{
  return mSuccessors;
}


const QString &MarkovNode::token(void) const
{
  return mToken;
}


MarkovNode *MarkovNode::selectSuccessor(qreal p)
{
  foreach(MarkovEdge *edge, mSuccessors) {
    if (p > edge->p()) {
      return edge->node();
    }
  }
  return Q_NULLPTR;
}


QDebug operator<<(QDebug debug, const MarkovNode &node)
{
  QDebugStateSaver saver(debug);
  Q_UNUSED(saver);
  debug.nospace() << "(" << node.token() << " -> ";
  foreach(MarkovEdge *edge, node.successors()) {
    debug << edge->node()->token() << "@" << edge->p() << " ";
  }
  debug << ")";
  return debug;
}
