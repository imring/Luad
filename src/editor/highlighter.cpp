// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021-2022 Vitaliy Vorobets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "highlighter.hpp"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter{parent} {
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {QStringLiteral("\\bdo\\b"),    QStringLiteral("\\bend\\b"), QStringLiteral("\\btrue\\b"),
                                       QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bnil\\b"), QStringLiteral("\\binvalid\\b")};
    for (const QString &pattern: keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = keywordFormat;
        highlightingRules.append(rule);
    }

    // dec numbers
    numdecFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\\b(\\+\\-){0,1}\\d+(\\.\\d+){0,1}\\b"));
    rule.format  = numdecFormat;
    highlightingRules.append(rule);

    // bin numbers
    numbinFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\\b(\\+\\-){0,1}(0b)?[01]+\\b"));
    rule.format  = numbinFormat;
    highlightingRules.append(rule);

    // hex numbers
    numhexFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\\b(\\+\\-){0,1}(0x)?[0-9A-Fa-f]+\\b"));
    rule.format  = numhexFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format  = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("\\-\\-[^\n]*"));
    rule.format  = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::gray);
    commentStartExpression = QRegularExpression(QStringLiteral("\\-\\-\\[\\["));
    commentEndExpression   = QRegularExpression(QStringLiteral("\\]\\]"));
}

void Highlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule: qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match         = commentEndExpression.match(text, startIndex);
        int                     endIndex      = match.capturedStart();
        int                     commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}