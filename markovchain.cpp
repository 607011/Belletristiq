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
#include <QJsonValue>
#include <QJsonObject>
#include <QtAlgorithms>


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


bool MarkovChain::readFromTextFile(const QString &filename)
{
  static const QRegExp reTokens("(\\b[^\\s]+\\b)([\\.,;!:\\?\\(\\)»«\"'_])?", Qt::CaseSensitive, QRegExp::RegExp);
  mCancelled = false;
  QFileInfo fi(filename);
  if (fi.isReadable() && fi.isFile()) {
    QFile inFile(filename);
    if (inFile.open(QIODevice::ReadOnly)) {
      int totalSize = 0;
      QStringList tokens;
      while (!inFile.atEnd()) {
        const QString &line = QString::fromUtf8(inFile.readLine());
        int pos = 0;
        while ((pos = reTokens.indexIn(line, pos)) != -1) {
          const QString &t1 = reTokens.cap(1);
          if (reTokens.captureCount() > 0 && !t1.isEmpty()) {
            tokens << t1;
          }
          const QString &t2 = reTokens.cap(2);
          if (reTokens.captureCount() > 1 && !t2.isEmpty()) {
            tokens << t2;
          }
          totalSize += t1.length() + t2.length();
          pos += reTokens.matchedLength();
        }
      }
      emit progressRangeChanged(0, totalSize);
      if (!tokens.isEmpty()) {
        add(tokens);
      }
      inFile.close();
    }
  }
  return false;
}


bool MarkovChain::readFromJsonFile(const QString &filename)
{
  qDebug() << "MarkovChain::readFromJsonFile(" << filename << ")";
  bool ok = false;
  mCancelled = false;
  QFile inFile(filename);
  if (inFile.open(QIODevice::ReadOnly)) {
    QByteArray header(4, '\0');
    inFile.read(header.data(), 4);
    bool compressed = (header == FileHeader);
    qDebug() << "compressed:" << compressed;
    if (!compressed) {
      inFile.seek(0);
    }
    QByteArray jsonData = compressed ? qUncompress(inFile.readAll()) : inFile.readAll();
    qDebug() << jsonData.mid(0, 500);
    QJsonParseError jsonError;
    QJsonDocument json = QJsonDocument::fromJson(jsonData, &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
      QVariantMap nodes = json.toVariant().toMap(); // TODO ...
      foreach (QString token, nodes.keys()) {
        MarkovNode *newNode = MarkovNode::fromVariantMap(nodes[token].toMap());
        mNodeMap.insert(newNode->token(), newNode);
      }
      ok = true;
    }
    else {
      // TODO: ...
    }
    inFile.close();
    if (ok) {
      postProcess();
    }
  }
  return ok;
}


void MarkovChain::save(const QString &filename, bool compressed)
{
  QFile outFile(filename);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QJsonDocument json = QJsonDocument::fromVariant(toVariantMap());
    QByteArray data = json.toJson(QJsonDocument::Indented);
    if (compressed) {
      outFile.write(FileHeader);
      data = qCompress(data, 9);
    }
    outFile.write(data);
    outFile.close();
  }
}


void MarkovChain::add(const QStringList &tokenList)
{
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
    emit progressValueChanged(int(bytesProcessed));
  }
}


QVariantMap MarkovChain::toVariantMap(void)
{
  QVariantMap map;
  foreach (MarkovNode *node, mNodeMap) {
    map.unite(node->toVariantMap());
  }
  return map;
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
