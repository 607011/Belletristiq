/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include "markovnode.h"
#include "markovedge.h"

#include <QDebug>
#include <QtGlobal>
#include <algorithm>


MarkovNode::MarkovNode(const QString &token)
  : mToken(token)
{
  /* ... */
}


bool edgeLessThan(MarkovEdge *a, MarkovEdge *b) {
  return a->node()->token() < b->node()->token();
}


void MarkovNode::addSuccessor(MarkovNode *node)
{
  // NOTES:
  // - Both algorithms have almost identical runtime behavior independent of the number of Markov nodes and edges.
  // - The insertion algorithm is virtually superior to the linear one because it produces a sorted list of successors.
#define INSERTION 1
#if LINEAR
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
#elif INSERTION
  MarkovEdge soughtEdge(node);
  MarkovEdgeList::iterator i = std::lower_bound(mSuccessors.begin(), mSuccessors.end(), &soughtEdge, edgeLessThan);
  if (i == mSuccessors.end()) {
    mSuccessors.append(new MarkovEdge(node));
  }
  else if ((*i)->node() == node) {
     (*i)->increaseCount();
  }
  else {
    mSuccessors.insert(i, new MarkovEdge(node));
  }
#endif
}


void MarkovNode::addSuccessor(MarkovEdge *edge)
{
  MarkovEdgeList::iterator i = std::lower_bound(mSuccessors.begin(), mSuccessors.end(), edge, edgeLessThan);
  mSuccessors.insert(i, edge);
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


MarkovNode *MarkovNode::selectSuccessor(const qreal p)
{
  foreach(MarkovEdge *edge, mSuccessors) {
    if (p > edge->p()) {
      return edge->node();
    }
  }
  return Q_NULLPTR;
}


QString MarkovNode::toString(void) const
{
  QString result = mToken + ' ';
  for (MarkovEdgeList::const_iterator i = mSuccessors.constBegin(); i != mSuccessors.constEnd(); ++i) {
    const MarkovEdge *successor = *i;
    result.append(successor->toString());
    if (i < (mSuccessors.constEnd() - 1))
      result.append(' ');
  }
  return result;
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
