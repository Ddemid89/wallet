#include "add_transfer_window.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace  {
QString FormatOperation(Wallet& wallet, const Transaction_DEL* trns_ptr) {
    QString res = trns_ptr->Date().toString("dd.MM.yy")
                  + ": " + wallet.GetAccName(trns_ptr->AccountFromIdx());

    if (trns_ptr->Type() == TransactionType::Income) {
        res += " <--(";
    } else if (trns_ptr->Type() == TransactionType::Expense) {
        res += " ---(";
    } else {
        res += " ---[";
    }

    res += trns_ptr->Sum().StringAbs() + " руб.";

    if (trns_ptr->Type() == TransactionType::Income) {
        res += ")--- " + wallet.GetCatName(trns_ptr->ToIdx());
    } else if (trns_ptr->Type() == TransactionType::Expense) {
        res += ")--> " + wallet.GetCatName(trns_ptr->ToIdx());
    } else {
        res += "]--> " + wallet.GetAccName(trns_ptr->ToIdx());
    }

    return res;
}
}

AddTransferWindow::AddTransferWindow(Wallet& wallet, MainWindow& m_window, QWidget *parent)
                                                         : Widgets(wallet, m_window, parent)  {
    QLabel* lab = new QLabel("Последние переводы:");
    QHBoxLayout* date_layout = new QHBoxLayout;
    QPushButton* next_day = new QPushButton("Следующий день");
    QLabel* lab2 = new QLabel("Новый перевод:");
    QHBoxLayout* select = new QHBoxLayout;
    QPushButton* submit = new QPushButton("Добавить перевод");

    last_ops_->setEnabled(false);
    lab2->setFrameStyle(2);

    date_->setMaximumDate(QDate::currentDate().addDays(1));
    date_->setAlignment(Qt::AlignHCenter);
    date_->setDisplayFormat("dd MMMM yyyy");

    sum_->setMinimum(0.01);
    sum_->setMaximum(1000000000);
    sum_->setSuffix(" руб.");

    date_layout->addWidget(date_, 3);
    date_layout->addWidget(next_day, 1);

    layout_->addWidget(lab);
    layout_->addWidget(last_ops_);
    layout_->addWidget(lab2);
    layout_->addLayout(date_layout);

    select->addWidget(new QLabel("из:"),1);
    select->addWidget(from_, 4);
    select->addWidget(new QLabel("--в-->:"), 1);
    select->addWidget(to_, 4);

    layout_->addLayout(select);
    layout_->addWidget(sum_);
    layout_->addWidget(submit);

    connect(submit, SIGNAL(clicked()), this, SLOT(submit()));
    connect(next_day, &QPushButton::clicked, [this]{
        date_->setDate(date_->date().addDays(1));
    });

    FillData();

    setLayout(layout_);
}

void AddTransferWindow::submit() {
    if (accs_idxs_.size() < 2) {
        QMessageBox::warning(0, "Ошибка", "Нужно как минимум два счета для перевода!");
        return;
    }

    transactions_manager::TransactionAdder adder;

    adder.from_idx = accs_idxs_.at(from_->currentIndex());
    adder.to_idx   = accs_idxs_.at(to_->currentIndex());

    adder.sum.FromDouble(sum_->value());
    adder.date = date_->date();

    adder.type = TransactionType::Transfer;

    if (!adder.IsValid()) {
        QMessageBox::warning(0, "Ошибка!", "Счета не могут совпадать!");
        return;
    }

    wallet_.AddTransaction(adder);
    FillLastOps();
}

void AddTransferWindow::FillData() {
    FillAccs();
    FillLastOps();
    sum_->setValue(0.01);
    date_->setDate(QDate::currentDate().addMonths(-1));
}

void AddTransferWindow::FillAccs() {
    const auto& accs = wallet_.GetAccounts();

    from_->clear();
    to_->clear();
    accs_idxs_.clear();

    for (auto& acc : accs) {
        if (!acc->IsDeleted()) {
            from_->addItem(acc->GetName());
            to_->addItem(acc->GetName());
            accs_idxs_.push_back(acc->GetIndex());
        }
    }
}

void AddTransferWindow::FillLastOps() {
    last_ops_->clear();

    auto ops = wallet_.GetTransfers(15, false);

    for (auto op : ops) {
        last_ops_->addItem(FormatOperation(wallet_, op));
    }
}

void AddTransferWindow::showEvent(QShowEvent*) {
    FillData();
}
