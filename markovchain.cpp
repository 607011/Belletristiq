/*
 * Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 *
 */

#include "markovchain.h"
#include <QRegExp>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QtAlgorithms>


const QByteArray MarkovChain::FileHeader("MRKV", 4);


MarkovChain::MarkovChain(void)
{
  /* ... */
}


void MarkovChain::postProcess(void)
{
  foreach (MarkovNode *node, mNodeMap) {
    node->calcProbabilities();
  }
}


void MarkovChain::clear(void)
{
  mNodeMap.clear();
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
  static const QRegExp reTokens("(\\b[^\\s]+\\b)([\\.,;!:\\?\\)])?", Qt::CaseSensitive, QRegExp::RegExp);
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
    postProcess();
    return true;
  }
  return false;
}


bool MarkovChain::readFromJsonFile(const QString &filename)
{
  qDebug() << "MarkovChain::readFromJsonFile(" << filename << ")";
  bool ok = false;
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
  foreach (QString token, tokenList) {
    MarkovNode *curr = mNodeMap.value(token, Q_NULLPTR);
    if (curr == Q_NULLPTR) {
      curr = new MarkovNode(token);
      mNodeMap.insert(curr->token(), curr);
    }
    if (prev != Q_NULLPTR) {
      prev->addSuccessor(curr);
    }
    prev = curr;
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
