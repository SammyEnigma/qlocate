#include "dragawarelistwidget.h"
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>

DragAwareListWidget::DragAwareListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

QMimeData * DragAwareListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
    QList<QUrl> urls;
    foreach (QListWidgetItem *ii, items)
        urls.push_back(QUrl::fromLocalFile(ii->data(Qt::ToolTipRole).toString()));

    QMimeData* data = new QMimeData;
    data->setUrls(urls);

    return data;
}

void DragAwareListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy))
    {
        QStringList strings;
        foreach (QListWidgetItem* ii, selectedItems())
            strings << ii->data(Qt::ToolTipRole).toString();

        QApplication::clipboard()->setText(strings.join("\n"));
    }
    QListWidget::keyPressEvent(event);
}
