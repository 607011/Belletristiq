/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */

#include "markovchain.h"



MarkovChain::MarkovChain(void)
{
  /* ... */
}


void MarkovChain::postProcess(void)
{
  foreach (MarkovNode *node, mNodeList) {
    node->calcProbabilities();
  }
}


int MarkovChain::count(void) const
{
  return mNodeList.count();
}


MarkovNode *MarkovChain::at(int idx)
{
  return mNodeList.at(idx);
}


const MarkovChain::MarkovNodeList &MarkovChain::nodes(void) const
{
  return mNodeList;
}


bool MarkovChain::find(const QString &token, MarkovNodeList::iterator &i)
{
  for (i = mNodeList.begin(); i != mNodeList.end(); ++i) {
    if (token == (*i)->token()) {
      return true;
    }
    else if (token < (*i)->token()) {
      return false;
    }
  }
  return false;
}


void MarkovChain::add(const QStringList &tokenList)
{
  MarkovNode *prev = Q_NULLPTR;
  foreach (QString token, tokenList) {
    MarkovNodeList::iterator idx;
    bool contained = find(token, idx);
    MarkovNode *curr = *idx;
    if (!contained) {
      curr = new MarkovNode(token);
      mNodeList.insert(idx, curr);
    }
    if (prev != Q_NULLPTR) {
      prev->addSuccessor(curr);
    }
    prev = curr;
  }
}



QDebug operator<<(QDebug debug, const MarkovChain &chain)
{
  QDebugStateSaver saver(debug);
  Q_UNUSED(saver);
  debug.nospace() << "MarkovChain@{";
  foreach(MarkovNode *node, chain.nodes()) {
    debug.noquote() << *node << " ";
  }
  debug.nospace() << "}";
  return debug;
}
