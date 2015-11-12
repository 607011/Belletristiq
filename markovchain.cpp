/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include <QRegExp>
#include <QFile>

MarkovChain::MarkovChain(void)
  : mCurrentNodeId(0)
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

void MarkovChain::readFromFile(const QString &filename)
{
  static const QRegExp reTokens("(\\b[^\\s]+\\b)([\\.,;!:\\?])?", Qt::CaseSensitive, QRegExp::RegExp);
  QFile inFile(filename);
  if (inFile.open(QIODevice::ReadOnly)) {
    QStringList tokens;
    while (!inFile.atEnd()) {
      const QString &line = QString::fromUtf8(inFile.readLine());
      int pos = 0;
      while ((pos = reTokens.indexIn(line, pos)) != -1) {
        if (reTokens.captureCount() > 0) {
          tokens << reTokens.cap(1);
        }
        if (reTokens.captureCount() > 1 && !reTokens.cap(2).isEmpty()) {
          tokens << reTokens.cap(2);
        }
        pos += reTokens.matchedLength();
      }
    }
    if (!tokens.isEmpty()) {
      add(tokens);
    }
    inFile.close();
  }
}


QVariantList MarkovChain::toVariantList(void) const
{
  QVariantList list;
  foreach (MarkovNode *node, mNodeList) {
    list.append(node->toVariantMap());
  }
  return list;
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
      curr = new MarkovNode(token, ++mCurrentNodeId);
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
