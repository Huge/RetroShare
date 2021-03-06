/***************************************************************************
 * Copyright (C) 2015-2017 by Savoir-faire Linux                           *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>          *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 **************************************************************************/

#include "UnseenContactItemDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QPixmap>
//#include <QSvgRenderer>
#include <QItemDelegate>

typedef QVector<unsigned int>                                       VectorUInt                    ;
// Client
#include "gui/common/UnseenContactSmartListModel.h"
#include "ringthemeutils.h"

#include <ciso646>

UnseenContactItemDelegate::UnseenContactItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{
//    QSvgRenderer svgRenderer(QString(":/images/icons/ic_baseline-search-24px.svg"));
//    searchIcon_ = new QPixmap(QSize(sizeImage_, sizeImage_));
//    searchIcon_->fill(Qt::transparent);
//    QPainter pixPainter(searchIcon_);
//    svgRenderer.render(&pixPainter);
}

void
UnseenContactItemDelegate::paint(QPainter* painter
                        , const QStyleOptionViewItem& option
                        , const QModelIndex& index
                        ) const
{
    QStyleOptionViewItem opt(option);
    painter->setRenderHint(QPainter::Antialiasing, true);

    bool selected = false;
    if (option.state & QStyle::State_Selected) {
        selected = true;
        opt.state ^= QStyle::State_Selected;
    }

    QRect rect_(opt.rect.left() - 2*dx_,opt.rect.top(), opt.rect.width() + 2*dx_, opt.rect.height());
    auto rowHighlight = highlightMap_.find(index.row());
    if (selected)
    {
        //painter->fillRect(option.rect, RingTheme::smartlistSelection_);
        painter->fillRect(rect_, RingTheme::smartlistSelection_);
    }

    QRect &rect = opt.rect;

    opt.decorationSize = QSize(sizeImage_, sizeImage_);
    opt.decorationPosition = QStyleOptionViewItem::Left;
    opt.decorationAlignment = Qt::AlignCenter;
    //QRect rectAvatar(dx_ + rect.left(), rect.top() + dy_, sizeImage_, sizeImage_);
    QRect rectAvatar(rect.left() - dx_, rect.top() + dy_, sizeImage_, sizeImage_);

    // Avatar drawing
    drawDecoration(painter, opt, rectAvatar,
                   QPixmap::fromImage(index.data(Qt::DecorationRole).value<QImage>())
                   .scaled(sizeImage_, sizeImage_, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Presence indicator
    QString statusStr =  index.data(static_cast<int>(UnseenContactSmartListModel::Role::Presence)).value<QString>();
    if (statusStr != "no-status")
    {
        qreal radius = sizeImage_ / 6;
        QPainterPath outerCircle, innerCircle;
        QPointF center(rectAvatar.right() - radius + 2, (rectAvatar.bottom() - radius) + 1 + 2);
        qreal outerCRadius = radius;
        qreal innerCRadius = outerCRadius * 0.75;
        outerCircle.addEllipse(center, outerCRadius, outerCRadius);
        innerCircle.addEllipse(center, innerCRadius, innerCRadius);
        painter->fillPath(outerCircle, Qt::white);
        if (statusStr == "offline")
        {
            painter->fillPath(innerCircle, RingTheme::grey_);
        }
        else if (statusStr == "idle")
        {
            painter->fillPath(innerCircle, RingTheme::urgentOrange_);
        }
        else if (statusStr == "online")
        {
            painter->fillPath(innerCircle, RingTheme::presenceGreen_);
        }
        else if (statusStr == "away")
        {
            painter->fillPath(innerCircle, RingTheme::urgentOrange_);
        }
        else if (statusStr == "busy")
        {
            painter->fillPath(innerCircle, RingTheme::red_);
        }
    }

    paintConversationItem(painter, option, rect, index,
                          false);

}

QSize
UnseenContactItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, cellHeight_);
}

void
UnseenContactItemDelegate::paintConversationItem(QPainter* painter,
                                                const QStyleOptionViewItem& option,
                                                const QRect& rect,
                                                const QModelIndex& index,
                                                const bool isTemporary) const
{
    //Q_UNUSED(option);
    QFont font(painter->font());
    QPen pen(painter->pen());
    painter->setPen(pen);

    int infoTextWidthModifier = 0;
    int infoText2HeightModifier = 0;
    //auto scalingRatio = MainWindow::instance().getCurrentScalingRatio();
    auto scalingRatio = 1.0;
    if (scalingRatio > 1.0) {
        font.setPointSize(fontSize_ - 2);
        infoTextWidthModifier = 12;
        infoText2HeightModifier = 2;
    } else {
        font.setPointSize(fontSize_);
        infoTextWidthModifier = 10;
    }

    //auto leftMargin = dx_ + sizeImage_ + dx_;
    auto leftMargin = sizeImage_;
    auto rightMargin = dx_ + 4;

    auto topMargin = 4;
    auto bottomMargin = 8;

    int rect1Width;
    rect1Width = rect.width() - leftMargin - 4*rightMargin - dx_;

    QRect rectName1(rect.left() + leftMargin,
                    rect.top()   + topMargin,
                    rect1Width,
                    rect.height() / 2 - 2);

    QRect rectName2(rectName1.left(),
                    rectName1.top() + rectName1.height() - infoText2HeightModifier,
                    rectName1.width(),
                    rectName1.height() - bottomMargin + infoText2HeightModifier);

    QFontMetrics fontMetrics(font);

    // The name is displayed at the avatar's right
    QString nameStr = index.data(static_cast<int>(UnseenContactSmartListModel::Role::DisplayName)).value<QString>();
    if (!nameStr.isNull()) {
        font.setItalic(false);
        font.setBold(true);
        pen.setColor(RingTheme::lightBlack_);
        painter->setPen(pen);
        painter->setFont(font);
        QString elidedNameStr = fontMetrics.elidedText(nameStr, Qt::ElideRight, rectName1.width());
        painter->drawText(rectName1, Qt::AlignVCenter | Qt::AlignLeft, elidedNameStr);
    }

    // Display the ID under the name
    QString idStr = index.data(static_cast<int>(UnseenContactSmartListModel::Role::DisplayID)).value<QString>();
    if (idStr != nameStr && !idStr.isNull()) {
        font.setItalic(false);
        font.setBold(false);
        pen.setColor(RingTheme::grey_);
        painter->setPen(pen);
        painter->setFont(font);
        idStr = fontMetrics.elidedText(idStr, Qt::ElideRight, rectName2.width());
        painter->drawText(rectName2, Qt::AlignVCenter | Qt::AlignLeft, idStr);
    }

    if (isTemporary) {
        return;
    }

    QRect rectInfo1(rectName1.left() + rectName1.width(),
                    rect.top() + topMargin,
                    4*rightMargin,
                    rect.height() / 2 - 2);
//    QRect rectInfo1(rectName1.left() + rectName1.width(),
//                    rect.top() + topMargin,
//                    infoTextWidth_ - rightMargin + infoTextWidthModifier + dx_,
//                    rect.height() / 2 - 2);

    QRect rectInfo2(rectInfo1.left(),
                    rectInfo1.top() + rectInfo1.height() - infoText2HeightModifier,
                    rectInfo1.width(),
                    rectInfo1.height() - bottomMargin + infoText2HeightModifier + 4);

    // top-right: last interaction date/time
    QString lastUsedStr = index.data(static_cast<int>(UnseenContactSmartListModel::Role::LastInteractionDate)).value<QString>();
    if (!lastUsedStr.isNull()) {
        font.setItalic(false);
        font.setBold(true);
        pen.setColor(RingTheme::blue_);
        painter->setPen(pen);
        font.setPointSize(fontSize_ + 10);
        painter->setFont(font);
        lastUsedStr = fontMetrics.elidedText(lastUsedStr, Qt::ElideRight, rectInfo1.width());
        painter->drawText(rectInfo1, Qt::AlignVCenter | Qt::AlignRight, lastUsedStr);

        painter->restore();
    }

    // bottom-right: last interaction snippet
    QString interactionStr = index.data(static_cast<int>(UnseenContactSmartListModel::Role::LastInteraction)).value<QString>();
    if (!interactionStr.isNull() && interactionStr.length() > 0)
    {
        painter->save();
        font.setWeight(QFont::ExtraLight);
        interactionStr = interactionStr.simplified();

        font.setItalic(false);
        font.setBold(false);
        pen.setColor(RingTheme::grey_.darker(140));
        painter->setPen(pen);
        painter->setFont(font);
        // strip emojis if it's a call/contact type message
        VectorUInt emojiless;
        for (auto unicode : interactionStr.toUcs4()) {
            if (!(unicode >= 0x1F000 && unicode <= 0x1FFFF)) {
                emojiless.push_back(unicode);
            }
        }
        interactionStr = QString::fromUcs4(&emojiless.at(0), emojiless.size());

        interactionStr = fontMetrics.elidedText(interactionStr, Qt::ElideRight, rectInfo2.width());
        painter->drawText(rectInfo2, Qt::AlignVCenter | Qt::AlignRight, interactionStr);
        painter->restore();
    }
}
