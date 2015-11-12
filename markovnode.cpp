/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include "markovnode.h"
#include "markovedge.h"

#include <QtGlobal>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>


MarkovNode::MarkovNode(const QString &token, int id)
  : mToken(token)
  , mId(id)
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


const QVariantList &MarkovNode::preliminarySuccessors(void) const
{
  return mPreliminarySuccessors;
}


void MarkovNode::setSuccessors(const MarkovEdgeList &successors)
{
  mSuccessors = successors;
}


void MarkovNode::setPreliminarySuccessors(const QVariantList &successors)
{
  mPreliminarySuccessors = successors;
}


const QString &MarkovNode::token(void) const
{
  return mToken;
}


int MarkovNode::id(void) const
{
  return mId;
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


void MarkovNode::postProcess(const MarkovChain *chain)
{
  mSuccessors.clear();
  foreach (QVariant s, mPreliminarySuccessors) {
    QVariantMap successor = s.toMap();
    const int id = successor["node_id"].toInt();
    const int count = successor["count"].toInt();
    foreach (MarkovNode *node, chain->nodes()) {
      if (node->id() == id) {
        MarkovEdge *successor = new MarkovEdge(node, count);
        mSuccessors.append(successor);
        break;
      }
    }
  }
}


QVariantMap MarkovNode::toVariantMap(void) const
{
  QVariantMap map;
  map["token"] = mToken;
  map["id"] = mId;
  QVariantList successors;
  foreach (MarkovEdge *successor, mSuccessors) {
    successors.append(successor->toVariantMap());
  }
  map["successors"] = successors;
  return map;
}


MarkovNode *MarkovNode::fromVariantMap(const QVariantMap &map)
{
  MarkovNode *node = new MarkovNode(map["token"].toString(), map["id"].toInt());
  node->setPreliminarySuccessors(map["successors"].toList());
  return node;
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
