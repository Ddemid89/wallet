#include "cell_w.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSpacerItem>



namespace {
QLabel* GetLabel(const QString& txt, int w = 0) {
    QLabel* res = new QLabel(txt);
    if (w != 0) {
        res->setFixedWidth(w);
    }
    res->setFrameStyle(2);
    res->setAlignment(Qt::AlignCenter);
    auto fnt = res->font();
    fnt.setBold(true);
    res->setFont(fnt);
    return res;
}
}

CellWindow::CellWindow(Wallet& wallet, MainWindow& m_window, QWidget* parent) : Widgets(wallet, m_window, parent) {
    QHBoxLayout* head = new QHBoxLayout;

    layout_->addSpacerItem(new QSpacerItem(500, 10));
    layout_->setSpacing(0);
    head->setContentsMargins(0, 0, 0, 0);
    layout_->addLayout(head);
    layout_->addWidget(s_area_);

    s_area_->setWidget(container_);
    container_->setLayout(main_layout_);
    container_->setGeometry(0, 0, 766, 80);
    container_->setContentsMargins(0, 0, 0, 0);

    s_area_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

    main_layout_->setContentsMargins(0, 0, 0, 0);
    main_layout_->setAlignment(Qt::AlignTop);
    main_layout_->addLayout(cells_layout_);

    QHBoxLayout* button_layout = new QHBoxLayout;
    QPushButton* add_row = new QPushButton("+");
    QPushButton* del_row = new QPushButton("-");

    head->addWidget(GetLabel("Дата",      81));
    head->addWidget(GetLabel("Операция",  85));
    head->addWidget(GetLabel("Сумма",     120));
    head->addWidget(GetLabel("Откуда",     150));
    head->addWidget(GetLabel("Куда"));
    head->addSpacerItem(new QSpacerItem(15, 3));

    connect(add_row, SIGNAL(clicked()), SLOT(AddRow()));
    connect(del_row, SIGNAL(clicked()), SLOT(PopRow()));

    button_layout->setContentsMargins(600, 10, 20, 0);
    button_layout->addWidget(del_row);
    button_layout->addItem(new QSpacerItem(20, 10));
    button_layout->addWidget(add_row);
    cells_layout_->setContentsMargins(0, 0, 0, 0);
    cells_layout_->setSpacing(0);

    main_layout_->addLayout(button_layout);
    main_layout_->setSpacing(0);

}

void CellWindow::Deactivate() {
    if (rows_.empty()) {
        return;
    }

    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "Закрытие", "Сохранить введенные транзакции?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (auto widget : rows_) {
            AddTransaction(widget->Get());
        }
    }

    for (auto widget : rows_) {
        cells_layout_->removeWidget(widget);
        delete widget;
    }
    rows_.clear();
}

void CellWindow::showEvent([[maybe_unused]]QShowEvent *event) {
    if (wallet_.GetAccounts().empty()) {
        QMessageBox::warning(0, "Ошибка", "Для создания транзакции должен существовать хотя-бы один счет и хотя-бы одна категория трат.");
        back_->click();
        return;
    }
}

void CellWindow::AddRow() {
    QDate date = QDate::currentDate();
    size_t acc_idx = 0;
    size_t cat_idx = 0;
    size_t op_idx  = 0;

    if (!rows_.empty()) {
        date    = rows_.back()->GetDate();
        acc_idx = rows_.back()->GetAccId();
        cat_idx = rows_.back()->GetCatId();
        op_idx  = rows_.back()->GetOp();
    }

    rows_.emplaceBack(new Row(wallet_, date, acc_idx, cat_idx, op_idx));
    cells_layout_->addWidget(rows_.back());

    container_->setGeometry(0, 0, 766, 70 + 23 * rows_.size());
}

void CellWindow::PopRow() {
    if (rows_.empty()) {
        return;
    }
    cells_layout_->removeWidget(rows_.back());
    delete rows_.back();
    rows_.pop_back();
}

void CellWindow::AddTransaction(CellTransaction&& ct) {
    if (ct.type == OpType::Transfer) {
        transactions_manager::TransferAdder adder;
        adder.date     = ct.date;
        adder.from_idx = ct.from;
        adder.to_idx   = ct.to;
        adder.sum      = ct.sum;
        Q_ASSERT(adder.IsValid());
        wallet_.AddTransfer(adder);
    } else {
        transactions_manager::TransactionAdder adder;
        adder.date     = ct.date;
        adder.acc_idx = ct.from;
        adder.cat_idx  = ct.to;
        adder.sum      = ct.sum;
        adder.inc      = ct.type == OpType::Inc;
        wallet_.AddTransaction(adder);
    }
}

Row::Row(Wallet &wallet, QDate date, size_t acc, size_t cat, size_t op, QWidget *parent)
    : QWidget{parent}, wallet_{wallet} {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    sum_->setMinimum(0.01);
    sum_->setMaximum(1000000.);
    sum_->setSuffix(" руб.");
    sum_->setFixedWidth(120);

    connect(op_, &QComboBox::currentIndexChanged, this, &Row::ChangeOp);
    op_->addItem("Расход");
    op_->addItem("Доход");
    if (wallet_.GetAccounts().size() > 1) {
        op_->addItem("Перевод");
    }
    op_->setFixedWidth(85);
    op_->setCurrentIndex(op);

    acc_from_->setFixedWidth(150);

    connect(acc_cat_to_, &QComboBox::currentIndexChanged, this, &Row::ChangeCat);
    connect(acc_from_,   &QComboBox::currentIndexChanged, this, &Row::ChangeAcc);

    layout->addWidget(date_label_);
    layout->addWidget(op_);
    layout->addWidget(sum_);
    layout->addWidget(acc_from_);
    layout->addWidget(acc_cat_to_);

    date_label_->setAlignment(Qt::AlignCenter);
    date_label_->setFixedWidth(80);
    date_label_->setDisplayFormat("dd.MM.yy");
    date_label_->setDate(date);

    FillAcs(*acc_from_);

    acc_from_->setCurrentIndex(acc);

    acc_cat_to_->setCurrentIndex(cat);

    setContentsMargins(0, 0, 0, 0);

}

CellTransaction Row::Get() {
    CellTransaction res;
    res.date = date_label_->date();
    switch (op_->currentIndex()) {
    case 0:
        res.type = OpType::Dec;
        res.to   = cat_idx_.at(acc_cat_to_->currentIndex());
        break;
    case 1:
        res.type = OpType::Inc;
        res.to   = cat_idx_.at(acc_cat_to_->currentIndex());
        break;
    case 2:
        res.type = OpType::Transfer;
        res.to   = acc_idx_.at(acc_cat_to_->currentIndex());
        break;
    default:
        throw std::logic_error("Something wrong in Row::Get()");
        break;
    }

    res.sum  = sum_->value();
    res.from = acc_idx_.at(acc_from_->currentIndex());

    return res;
}

QDate Row::GetDate() const {
    return date_label_->date();
}

size_t Row::GetAccId() const {
    return acc_from_->currentIndex();
}

size_t Row::GetCatId() const {
    return acc_cat_to_->currentIndex();
}

size_t Row::GetOp() const {
    return op_->currentIndex();
}

void Row::ChangeOp() {
    uint op_id = op_->currentIndex();

    QPalette pal;

    if (op_id == 0) {
        pal.setColor(QPalette::Text, Qt::red);
        FillCats(false);
    } else if (op_id == 1) {
        pal.setColor(QPalette::Text, Qt::green);
        FillCats(true);
    } else {
        pal.setColor(QPalette::Text, Qt::blue);
        FillAcs(*acc_cat_to_);
        if (acc_from_->currentIndex() == 0) {
            acc_cat_to_->setCurrentIndex(1);
        }
    }

    sum_->setPalette(pal);
}

void Row::ChangeAcc() {
    if (op_->currentIndex() == 2) {
        if (acc_from_->currentIndex() == acc_cat_to_->currentIndex()) {
            if (acc_cat_to_->currentIndex() == 0) {
                acc_cat_to_->setCurrentIndex(1);
            } else {
                acc_cat_to_->setCurrentIndex(0);
            }
        }
    }
}

void Row::ChangeCat() {
    if (op_->currentIndex() == 2) {
        if (acc_from_->currentIndex() == acc_cat_to_->currentIndex()) {
            if (acc_from_->currentIndex() == 0) {
                acc_from_->setCurrentIndex(1);
            } else {
                acc_from_->setCurrentIndex(0);
            }
        }
    }
}


void Row::FillAcs(QComboBox &cb) {
    auto& accs = wallet_.GetAccounts();

    cb.clear();

    acc_idx_.clear();

    for (auto& acc : accs) {
        if (!acc->IsDeleted()) {
            cb.addItem(acc->GetName());
            acc_idx_.push_back(acc->GetIndex());
        }
    }
}

void Row::FillCats(bool inc) {
    auto cats = wallet_.GetCategories(inc);

    acc_cat_to_->clear();
    cat_idx_.clear();

    for (auto& cat : cats) {
        acc_cat_to_->addItem(QString(cat.indent, ' ') + cat.name);
        cat_idx_.push_back(cat.idx);
    }

}
