/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#include "markovedge.h"

MarkovEdge::MarkovEdge(MarkovNode *node)
  : mNode(node)
  , mCount(1)
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
