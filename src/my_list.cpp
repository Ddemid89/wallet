#include "my_list.h"

#include <QMouseEvent>
#include <QKeyEvent>

MyList::MyList(QWidget *parent) : QWidget{parent} {
    QVBoxLayout* main_layout = new QVBoxLayout;

    MyLine* head = new MyLine("Дата", "Откуда", "Сумма", "Куда", "");
    head->SetInd(1);
    head->SetAlignment(Qt::AlignCenter);
    QHBoxLayout* inc_lyt = new QHBoxLayout;
    QHBoxLayout* dec_lyt = new QHBoxLayout;
    QHBoxLayout* tot_lyt = new QHBoxLayout;

    area_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    area_->setWidget(cont_);
    cont_->setLayout(lines_lyt_);

    cont_->setFixedWidth(766);

    lines_lyt_->setAlignment(Qt::AlignTop);
    lines_lyt_->setSpacing(0);
    lines_lyt_->setContentsMargins(0, 0, 0, 0);

    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    MakeBasementLine("Итого доходов:", inc_lyt, inc_);
    MakeBasementLine("Итого расходов:", dec_lyt, dec_);
    MakeBasementLine("Итоговый баланс:", tot_lyt, tot_);

    main_layout->addWidget(head);
    main_layout->addWidget(area_, 1);
    main_layout->addLayout(inc_lyt);
    main_layout->addLayout(dec_lyt);
    main_layout->addLayout(tot_lyt);

    setLayout(main_layout);
}

void MyList::clear() {
    while (!lines_.empty()) {
        auto line_ptr = lines_.back();
        lines_.pop_back();
        delete line_ptr;
    }
    cur_ = -1;
}

void MyList::addItem(QDate date, const QString &from, const QString &sum, const QString &to, const QString &desc, bool transfer, bool inc) {

    char first_char;
    char quote = '(';

    QString sum_res;

    if (transfer) {
        first_char = '-';
        quote = '[';
        sum_res = sum + " руб.]---->       ";
    } else if (inc) {
        first_char = '-';
        sum_res = sum + " руб.)---->       ";
    } else {
        first_char = '<';
        sum_res = sum + " руб.)-----       ";
    }

    sum_res = QString(first_char) + QString(30 - sum_res.size(), '-') + QString(quote) + sum_res;

    auto new_line = new MyLine{date.toString("dd.MM.yy"),
                               "   " + from,
                               sum_res,
                               to + "   ",
                               desc
    };
    lines_.push_back(new_line);
    lines_lyt_->addWidget(new_line);
    cont_->setFixedHeight(16 * lines_.size() + 1);
    connect(lines_.back(), &MyLine::doubleClicked, this, &MyList::itemDoubleClicked);
    connect(lines_.back(), &MyLine::clicked, [ind = lines_.size() - 1, this]{
        itemClicked(ind);
    });
}

int MyList::currentRow() {
    return cur_;
}

void MyList::SetSums(const QString &inc, const QString &dec, const QString &tot) {
    const auto ind = " руб." + QString(13, ' ');
    inc_->setText(inc + ind);
    dec_->setText(dec + ind);
    tot_->setText(tot + ind);
}

void MyList::keyPressEvent(QKeyEvent *event) {
    int new_ind;
    switch (event->key()) {
    case Qt::Key_Up:
        new_ind = qMax(0, cur_ - 1);
        itemClicked(new_ind);
        break;
    case Qt::Key_Down:
        new_ind = qMin(lines_.size() - 1, cur_ + 1);
        itemClicked(new_ind);
        break;
    case Qt::Key_Return:
        emit itemDoubleClicked();
        break;
    }
}

MyLine::MyLine(const QString& date, const QString& from, const QString& sum, const QString& to, const QString& desc) {
    QHBoxLayout* lyt = new QHBoxLayout;
    lyt->setContentsMargins(0, 0, 0, 0);
    lyt->setSpacing(0);

    date_->setText(date);
    from_->setText(from);
    sum_->setText(sum);
    to_->setText(to);

    date_->setFont(QFont(MONOSPACE_FONT));
    date_->setFixedSize(100, 15);
    date_->setAlignment(Qt::AlignCenter);

    from_->setFont(QFont(MONOSPACE_FONT));
    from_->setFixedSize(150, 15);
    from_->setAlignment(Qt::AlignLeft);

    sum_->setFixedSize(300, 15);
    sum_->setAlignment(Qt::AlignRight);

    to_->setFixedHeight(15);
    to_->setAlignment(Qt::AlignRight);

    lyt->addWidget(date_);
    lyt->addWidget(line1);
    lyt->addWidget(from_);
    lyt->addWidget(line2);
    lyt->addWidget(sum_);
    lyt->addWidget(line3);
    lyt->addWidget(to_, 1);

    date_->setAutoFillBackground(true);
    from_->setAutoFillBackground(true);
    sum_->setAutoFillBackground(true);
    to_->setAutoFillBackground(true);

    line1->setAutoFillBackground(true);
    line2->setAutoFillBackground(true);
    line3->setAutoFillBackground(true);

    if (desc != "") {
        setToolTip(desc);
    } else {
        setToolTip("Нет описания");
    }

    connect(date_, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(date_, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);

    connect(from_, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(from_, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);

    connect(sum_, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(sum_, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);

    connect(to_, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(to_, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);

    connect(line1, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(line1, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);
    connect(line2, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(line2, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);
    connect(line3, &ClickableLabel::clicked, this, &MyLine::clicked);
    connect(line3, &ClickableLabel::doubleClicked, this, &MyLine::doubleClicked);

    setLayout(lyt);
}

void MyLine::SetAlignment(Qt::Alignment date, Qt::Alignment from, Qt::Alignment sum, Qt::Alignment to) {
    date_->setAlignment(date);
    from_->setAlignment(from);
    sum_->setAlignment(sum);
    to_->setAlignment(to);
}

void MyLine::SetAlignment(Qt::Alignment common) {
    SetAlignment(common, common, common, common);
}

void MyLine::SetInd(size_t ind) {
    date_->setFixedWidth(100 + ind);
}

void MyLine::SetColor(const QColor &color) {
    QPalette pl;
    pl.setColor(QPalette::Window, color);
    date_->setPalette(pl);
    from_->setPalette(pl);
    sum_->setPalette(pl);
    to_->setPalette(pl);
    line1->setPalette(pl);
    line2->setPalette(pl);
    line3->setPalette(pl);
}

void MyLine::SetActive(bool act) {
    setFrameStyle(act);
    date_->setFixedWidth(99 + !act);
}

void MyList::MakeBasementLine(const QString &txt, QHBoxLayout *lyt, QLabel *label) {
    lyt->setContentsMargins(0, 0, 0, 0);
    lyt->setSpacing(0);

    auto beg = new QLabel;
    auto info = new QLabel(txt);
    auto end = new QLabel;

    beg->setFixedWidth(101);
    info->setFixedWidth(150);
    label->setFixedWidth(300);

    info->setAlignment(Qt::AlignRight);
    label->setAlignment(Qt::AlignRight);

    lyt->addWidget(beg);
    lyt->addWidget(new QLabel("|"));
    lyt->addWidget(info);
    lyt->addWidget(new QLabel("|"));
    lyt->addWidget(label);
    lyt->addWidget(new QLabel("|"));
    lyt->addWidget(end, 1);
}

void MyList::itemClicked(int i) {
    if (i == cur_) {
        return;
    }
    QColor dis = QColor(251, 251, 251);
    QColor act = QColor(204, 204, 255);
    if (cur_ != -1) {
        lines_.at(cur_)->SetColor(dis);
        lines_.at(cur_)->SetActive(false);
    }
    cur_ = i;

    lines_.at(cur_)->SetColor(act);
    lines_.at(cur_)->SetActive(true);

    setFocus();
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(event->globalPos());
    }
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
}
