#ifndef CLICKABLE_LABEL_H
#define CLICKABLE_LABEL_H

#include <QLabel>

class ClickableLabel : public QLabel {
    Q_OBJECT
    using QLabel::QLabel;
signals:
    void clicked(const QPoint& point);
    void doubleClicked();
public:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // CLICKABLE_LABEL_H
