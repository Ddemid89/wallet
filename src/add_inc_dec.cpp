#include "add_inc_dec.h"
#include "mainwindow.h"
#include "model.h"

#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
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

AddIncDec::AddIncDec(Wallet& wallet, MainWindow& m_window, bool arrive, QWidget* parent)
                                                        : Widgets(wallet, m_window, parent)
                                                        , arrive_(arrive) {
    DrawInterface();
    FillData();
}

void AddIncDec::commit() {
    if (accs_idxs_.empty()) {
        QMessageBox::warning(0, "Ошибка", "Нельзя зарегистрировать транзакцию! Нет ни одного счета!");
        return;
    }

    transactions_manager::TransactionAdder transact;
    transact.date = date_w->date();
    transact.from_idx = accs_idxs_.at(account_w->currentIndex());
    transact.to_idx = cats_idxs_.at(target_w->currentIndex());
    transact.sum.FromDouble(arrive_ ? sum_w->value() : -sum_w->value());
    transact.type = arrive_ ? TransactionType::Income : TransactionType::Expense;

    wallet_.AddTransaction(transact);
    emit(m_window_.show_status("Транзакция добавлена"));
    FillLastOps();
}

void AddIncDec::FillData() {
    FillAccounts();
    FillTarget();
    date_w->setDate(QDate::currentDate().addMonths(-1));
    sum_w->setValue(0.01);
    FillLastOps();
}

void AddIncDec::showEvent(QShowEvent*) {
    FillData();
}

void AddIncDec::DrawInterface() {
    QFormLayout* form_layout = new QFormLayout;
    QPushButton* commit = new QPushButton;
    QPushButton* next_day = new QPushButton("Следующий день");
    QLabel* lab = new QLabel(arrive_ ? "Последние доходы:" : "Последние расходы:");
    QLabel* lab2 = new QLabel("Новая транзакция:");

    date_w->setMaximumDate(QDate::currentDate().addDays(1));

    date_w->setDisplayFormat("dd MMMM yyyy");

    sum_w->setDecimals(2);
    sum_w->setMaximum(1000000000);
    sum_w->setMinimum(0.01);
    sum_w->setSuffix(" руб.");

    last_ops_->setEnabled(false);

    date_w->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    lab2->setFrameStyle(2);

    QPalette palette;
    if (arrive_) {
        palette.setColor(QPalette::Text, Qt::green);
        commit->setText("Добавить доход");
    } else {
        palette.setColor(QPalette::Text, Qt::red);
        commit->setText("Добавить расход");
    }
    sum_w->setPalette(palette);

    connect(commit, SIGNAL(clicked()), this, SLOT(commit()));
    connect(next_day, &QPushButton::clicked, [this](){
        date_w->setDate(date_w->date().addDays(1));
    });

    form_layout->addRow(lab);
    form_layout->addRow(last_ops_);
    form_layout->addRow(lab2);
    form_layout->addRow(date_w);
    form_layout->addRow(next_day);
    form_layout->addRow(new QLabel("Счет:"), account_w);
    form_layout->addRow(new QLabel("Категория:"), target_w);
    form_layout->addRow(new QLabel("Сумма:"), sum_w);
    form_layout->addRow(commit);

    FillData();

    layout_->addLayout(form_layout);

    setLayout(layout_);
}

void AddIncDec::FillAccounts() {
    const auto& accs = wallet_.GetAccounts();

    account_w->clear();

    accs_idxs_.clear();
    accs_idxs_.reserve(accs.size());

    for (auto& acc : accs) {
        if (!acc->IsDeleted()) {
            account_w->addItem(acc->GetName());
            accs_idxs_.push_back(acc->GetIndex());
        }
    }

}

void AddIncDec::FillTarget() {
    const auto cats = wallet_.GetCategories(arrive_);

    target_w->clear();

    cats_idxs_.clear();
    cats_idxs_.reserve(cats.size());

    for (const auto& cat : cats) {
        target_w->addItem(QString(cat.indent * 2, ' ') + cat.name);
        cats_idxs_.push_back(cat.idx);
    }
}

void AddIncDec::FillLastOps() {
    last_ops_->clear();

    auto ops = wallet_.GetIncDecTransacts(arrive_, 15, false);

    for (auto op : ops) {
        last_ops_->addItem(FormatOperation(wallet_, op));
    }
}
