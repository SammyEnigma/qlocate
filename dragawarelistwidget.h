#ifndef DRAGAWARELISTWIDGET_H
#define DRAGAWARELISTWIDGET_H

#include <QListWidget>

class DragAwareListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit DragAwareListWidget(QWidget *parent = 0);

protected:
    virtual QMimeData * mimeData(const QList<QListWidgetItem*> items) const;

signals:

public slots:

};

#endif // DRAGAWARELISTWIDGET_H
