/* Copyright (c) 2015 Oliver Lau <oliver@ersatzworld.net> - All rights reserved. */


#include <random>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QDateTime>
#include <QScopedPointer>

#include "markovnode.h"
#include "markovnode.h"
#include "markovchain.h"

int main(int argc, char *argv[])
{
  static const QRegExp reTokens("(\\b[^\\s]+\\b)([\\.,;!:\\?])?", Qt::CaseSensitive, QRegExp::RegExp);

  QScopedPointer<MarkovChain> markovChain(new MarkovChain);

  for (int fileIdx = 1; fileIdx < argc; ++fileIdx) {
    qDebug() << argv[fileIdx];
    QFile inFile(argv[fileIdx]);
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
        markovChain->add(tokens);
      }
      inFile.close();
    }
  }

  markovChain->postProcess();

  std::mt19937 rng;
  rng.seed(QDateTime::currentDateTimeUtc().toTime_t());
  std::uniform_real_distribution<qreal> pDist(0.0, 1.0);
  std::uniform_int_distribution<int> nDist(0, markovChain->count() - 1);

  MarkovNode *node = Q_NULLPTR;
  int N = 400;
  QStringList result;
  while (--N > 0) {
    if (node == Q_NULLPTR) {
      node = markovChain->at(nDist(rng));
    }
    const QString &token = node->token();
    result << token;
    node = node->selectSuccessor(pDist(rng));
  }
  qDebug() << result.join(' ');

  return 0;
}
