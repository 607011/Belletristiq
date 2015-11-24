/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */


#include "markovedge.h"
#include "markovnode.h"

MarkovEdge::MarkovEdge(MarkovNode *node, int count)
  : mNode(node)
  , mCount(count)
  , mProbability(1.0)
{
  /* ... */
}


MarkovNode *MarkovEdge::node(void)
{
  return mNode;
}


int MarkovEdge::count(void) const
{
  return mCount;
}


void MarkovEdge::setCount(int count)
{
  mCount = count;
}


qreal MarkovEdge::p(void) const
{
  return mProbability;
}


void MarkovEdge::setProbability(qreal p)
{
  mProbability = p;
}


void MarkovEdge::increaseCount(void)
{
  ++mCount;
}


QString MarkovEdge::toString(void) const
{
  return QString("%1 %2").arg(mCount).arg(mNode->token());
}
