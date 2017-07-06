/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include "markovedge.h"

#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>

const QByteArray MarkovChain::FileHeader("MRKV", 4);


MarkovChain::MarkovChain(void)
  : mCancelled(false)
{
  /* ... */
}


void MarkovChain::postProcess(void)
{
  if (!mCancelled) {
    foreach (MarkovNode *node, mNodeMap) {
      node->calcProbabilities();
    }
  }
}


void MarkovChain::clear(void)
{
  mNodeMap.clear();
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
  return mNodeMap.count();
}


MarkovNode *MarkovChain::at(int idx)
{
  MarkovNodeMap::iterator node = mNodeMap.begin() + idx;
  return *node;
}


bool MarkovChain::readFromMarkovFile(const QString &filename)
{
  qDebug() << "MarkovChain::readFromMarkovFile(" << filename << ")";
  bool ok = false;
  mCancelled = false;
  mSignalTimer.start();
  QFile inFile(filename);
  if (inFile.open(QIODevice::ReadOnly)) {
    QByteArray header(4, '\0');
    inFile.read(header.data(), 4);
    bool compressed = (header == FileHeader);
    if (!compressed) {
      inFile.seek(0);
    }
    QString data = compressed ? qUncompress(inFile.readAll()) : inFile.readAll();
    inFile.close();
    QStringList lines = data.split('\n');
    // 1st pass: add nodes without successors
    foreach (QString line, lines) {
      QStringList m = line.split(' ');
      MarkovNode *newNode = Q_NULLPTR;
      if (!m.isEmpty()) {
        newNode = new MarkovNode(m.first());
        mNodeMap.insert(newNode->token(), newNode);
      }
    }
    // 2nd pass: add successors to nodes
    foreach (QString line, lines) {
      const QStringList &strEdge = line.split(' ', QString::SkipEmptyParts);
      if (!strEdge.isEmpty()) {
        const QString &token = strEdge.first();
        Q_ASSERT(mNodeMap.keys().contains(token));
        MarkovNode *node = mNodeMap[token];
        for (int i = 1; i < strEdge.size(); i += 2) {
          const int count = strEdge.at(i).toInt(&ok);
          if (ok) {
            const QString &token = strEdge.at(i + 1);
            MarkovNode *refNode = mNodeMap[token];
            MarkovEdge *edge = new MarkovEdge(refNode, count);
            node->addSuccessor(edge);
          }
        }
      }
    }
    postProcess();
  }
  return ok;
}


void MarkovChain::save(const QString &filename)
{
  QFile outFile(filename);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QByteArray data = toString().toUtf8();
    if (filename.endsWith('z')) {
      outFile.write(FileHeader);
      data = qCompress(data, 9);
    }
    outFile.write(data);
    outFile.close();
  }
}


void MarkovChain::add(const QStringList &tokenList)
{
  if (!tokenList.isEmpty()) {
    MarkovNode *prev = Q_NULLPTR;
    int bytesProcessed = 0;
    foreach (QString token, tokenList) {
      if (mCancelled)
        break;
      MarkovNode *curr = mNodeMap.value(token, Q_NULLPTR);
      if (curr == Q_NULLPTR) {
        curr = new MarkovNode(token);
        mNodeMap.insert(curr->token(), curr);
      }
      if (prev != Q_NULLPTR) {
        prev->addSuccessor(curr);
      }
      prev = curr;
      bytesProcessed += token.length();
      if (mSignalTimer.elapsed() > 1000 / 30) {
        emit progressValueChanged(int(bytesProcessed));
        mSignalTimer.restart();
      }
    }
  }
}


QString MarkovChain::toString(void) const
{
  QString result;
  foreach (MarkovNode *node, mNodeMap) {
    result.append(node->toString()).append('\n');
  }
  return result;
}


const MarkovChain::MarkovNodeMap &MarkovChain::nodes(void) const
{
  return mNodeMap;
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
