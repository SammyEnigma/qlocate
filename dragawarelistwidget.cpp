#include "dragawarelistwidget.h"
#include <QtCore/QMimeData>
#include <QtCore/QUrl>

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
