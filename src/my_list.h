#ifndef MY_LIST_H
#define MY_LIST_H

#include <QWidget>
#include <QDate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

#ifdef Q_OS_WINDOWS
    const auto MONOSPACE_FONT = "Courier";
#else
    const auto MONOSPACE_FONT = "Monospace";
#endif

class ClickableLabel : public QLabel {
Q_OBJECT
    using QLabel::QLabel;
signals:
    void clicked();
    void doubleClicked();
public:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *event);
};



class MyLine : public QFrame {
    Q_OBJECT
public:
    MyLine(const QString& date, const QString& from, const QString& sum, const QString& to, const QString& desc);
    void SetAlignment(Qt::Alignment date, Qt::Alignment from, Qt::Alignment sum, Qt::Alignment to);
    void SetAlignment(Qt::Alignment common);
    void SetInd(size_t ind);
    QPalette t;
    void SetColor(const QColor& color);
    void SetActive(bool act);
signals:
    void clicked();
    void doubleClicked();
private:
    ClickableLabel* date_ = new ClickableLabel;
    ClickableLabel* from_ = new ClickableLabel;
    ClickableLabel* sum_ = new ClickableLabel;
    ClickableLabel* to_ = new ClickableLabel;

    ClickableLabel* line1 = new ClickableLabel("|");
    ClickableLabel* line2 = new ClickableLabel("|");
    ClickableLabel* line3 = new ClickableLabel("|");
};



class MyList : public QWidget {
    Q_OBJECT

signals:
    void itemDoubleClicked();
public:
    explicit MyList(QWidget *parent = nullptr);
    void clear();
    void addItem(QDate date, const QString& from, const QString& sum, const QString& to, const QString& desc, bool transfer, bool inc);
    int currentRow();
    void SetSums(const QString& inc, const QString& dec, const QString& tot);
    void keyPressEvent(QKeyEvent *event);
private:
    void MakeBasementLine(const QString& txt, QHBoxLayout* lyt, QLabel* label);
    void itemClicked(int i);

    int cur_ = -1;

    QScrollArea* area_ = new QScrollArea;
    QWidget* cont_ = new QWidget;
    QVBoxLayout* lines_lyt_ = new QVBoxLayout;

    QLabel* inc_ = new QLabel;
    QLabel* dec_ = new QLabel;
    QLabel* tot_ = new QLabel;

    QVector<MyLine*> lines_;
};

#endif // MY_LIST_H
