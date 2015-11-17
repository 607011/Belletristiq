/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutexLocker>


MarkovChain::MarkovChain(void)
  : mCurrentNodeId(0)
  , mCancelled(false)
{
  /* ... */
}


void MarkovChain::postProcess(void)
{
  if (!mCancelled) {
    foreach (MarkovNode *node, mNodeList) {
      node->calcProbabilities();
    }
  }
}


void MarkovChain::clear(void)
{
  mCurrentNodeId = 0;
  mNodeList.clear();
}


bool MarkovChain::isCancelled(void) const
{
  return mCancelled;
}


void MarkovChain::cancel(void)
{
  mCancelled = true;
}


int MarkovChain::count(void) const
{
  return mNodeList.count();
}


MarkovNode *MarkovChain::at(int idx)
{
  return mNodeList.at(idx);
}


void MarkovChain::readFromTextFile(const QString &filename)
{
  static const QRegExp reTokens("(\\b[^\\s]+\\b)([\\.,;!:\\?\\)])?", Qt::CaseSensitive, QRegExp::RegExp);
  mCancelled = false;
  QFileInfo fi(filename);
  if (fi.isReadable() && fi.isFile()) {
    QFile inFile(filename);
    if (inFile.open(QIODevice::ReadOnly)) {
      emit progressRangeChanged(0, int(QFileInfo(filename).size()));
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
}


void MarkovChain::readFromJsonFile(const QString &filename)
{
  mCancelled = false;
  QFile inFile(filename);
  if (inFile.open(QIODevice::ReadOnly)) {
    QByteArray jsonData = inFile.readAll();
    QJsonParseError jsonError;
    QJsonDocument json = QJsonDocument::fromJson(jsonData, &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
      QJsonArray nodes = json.array();
      foreach (QJsonValue node, nodes) {
        MarkovNode *newNode = MarkovNode::fromVariantMap(node.toObject().toVariantMap());
        mNodeList.append(newNode);
      }
      foreach (MarkovNode *node, mNodeList) {
        node->postProcess(this);
      }
    }
    inFile.close();
  }
}


void MarkovChain::add(const QStringList &tokenList)
{
  MarkovNode *prev = Q_NULLPTR;
  int bytesProcessed = 0;
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
    bytesProcessed += token.length();
    emit progressValueChanged(int(bytesProcessed));
    if (mCancelled) {
      break;
    }
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
