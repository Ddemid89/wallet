#include "transaction_editor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

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

TransactionEditor::TransactionEditor(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                            : Widgets(wallet, m_window, parent) {
    QHBoxLayout* date_select_ = new QHBoxLayout;
    QHBoxLayout* type_select_ = new QHBoxLayout;

    QPushButton* show_data = new QPushButton("Показать");

    type_->addItem("Всё");
    type_->addItem("Доходы");
    type_->addItem("Расходы");
    type_->addItem("Переводы");

    date_to_->setDate(QDate::currentDate());
    date_from_->setDate(QDate::currentDate().addDays(-7));

    date_to_->setDisplayFormat("dd MMMM yyyy");
    date_from_->setDisplayFormat("dd MMMM yyyy");

    date_to_->setMaximumDate(QDate::currentDate());
    date_from_->setMaximumDate(QDate::currentDate());

    date_select_->addWidget(new QLabel("Промежуток с: "));
    date_select_->addWidget(date_from_);
    date_select_->addWidget(new QLabel("по: "));
    date_select_->addWidget(date_to_);

    type_select_->addWidget(new QLabel("Операция: "));
    type_select_->addWidget(type_);
    type_select_->addWidget(new QLabel("Счет: "));
    type_select_->addWidget(acc_);
    type_select_->addWidget(new QLabel("Категория: "));
    type_select_->addWidget(target_);

    layout_->addLayout(date_select_);
    layout_->addLayout(type_select_);
    layout_->addWidget(show_data);
    layout_->addWidget(list_);

    setLayout(layout_);

    connect(date_from_, &QDateEdit::dateChanged, [this]{
        date_to_->setMinimumDate(date_from_->date());
    });
    connect(date_to_, &QDateEdit::dateChanged, [this]{
        date_from_->setMaximumDate(date_to_->date());
    });

    connect(list_, &QListWidget::itemDoubleClicked, this, &TransactionEditor::Edit);

    connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(FillTargets()));
    connect(show_data, SIGNAL(clicked()), this, SLOT(FillOps()));

    QHBoxLayout* btns_layout_ = new QHBoxLayout;

    QPushButton* del_btn = new QPushButton("Удалить");
    del_btn->setFixedWidth(120);
    btns_layout_->addWidget(del_btn);

    QPushButton* edit_btn = new QPushButton("Редактировать");
    edit_btn->setFixedWidth(120);
    btns_layout_->addWidget(edit_btn);

    layout_->addLayout(btns_layout_);

    connect(del_btn, SIGNAL(clicked()), SLOT(Delete()));
    connect(edit_btn, SIGNAL(clicked()), SLOT(Edit()));

    FillData();
}

void TransactionEditor::FillData() {
    FillAccs();
    FillTargets();
    FillOps();
}

void TransactionEditor::FillAccs() {
    const auto& accs = wallet_.GetAccounts();

    acc_->clear();
    acc_->addItem("Любой");

    accs_idxs_.clear();
    accs_idxs_.reserve(accs.size() + 1);
    accs_idxs_.push_back(0);

    for (auto& acc : accs) {
        acc_->addItem(acc->GetName());
        accs_idxs_.push_back(acc->GetIndex());
    }
}

void TransactionEditor::FillTargets() {
    const auto cats = GetCats();

    target_->clear();
    cats_idxs_.clear();

    for (auto& cat : cats) {
        target_->addItem(cat.name);
        cats_idxs_.push_back(cat.idx);
    }
}

void TransactionEditor::FillOps() {
    list_->clear();
    trns_idxs_.clear();

    QDate from = date_from_->date();
    QDate to = date_to_->date();

    TransactShowType type = TransactShowType::All;

    switch (type_->currentIndex()) {
    case 0:
        type = TransactShowType::All;
        break;
    case 1:
        type = TransactShowType::Income;
        break;
    case 2:
        type = TransactShowType::Expense;
        break;
    case 3:
        type = TransactShowType::Transfer;
        break;
    }

    size_t acc = accs_idxs_.at(acc_->currentIndex());
    size_t cat = 0;

    if (type != TransactShowType::Transfer) {
        cat = cats_idxs_.at(target_->currentIndex());
    }

    auto ops = wallet_.GetTransactFiltred(from, to, type, acc, cat);

    for (auto& op : ops) {
        list_->addItem(FormatOperation(wallet_, op) /*+ " (id: " + QString::number(op->Index()) + ")"*/ );
        trns_idxs_.push_back(op->Index());
    }
}

void TransactionEditor::Edit() {
    int idx = list_->currentRow();

    if (idx == -1) {
        return;
    }

    const Transaction_DEL* op = wallet_.FindTransact(trns_idxs_.at(idx));

    ModalEditor* me = new ModalEditor(op, wallet_);

    connect(me, SIGNAL(Updated()), SLOT(FillOps()));

    me->show();
}

void TransactionEditor::Delete() {
    int idx = list_->currentRow();

    if (idx == -1) {
        return;
    }

    size_t trns = trns_idxs_.at(idx);

    wallet_.DeleteTransaction(trns);
    FillOps();
}

const QVector<CategoryInfo> TransactionEditor::GetCats() const {
    switch(type_->currentIndex()) {
    case 0:
        target_->setEnabled(true);
        return wallet_.GetAllCategories();
    case 1:
        target_->setEnabled(true);
        return wallet_.GetCategories(true);
    case 2:
        target_->setEnabled(true);
        return wallet_.GetCategories(false);
    default:
        target_->setEnabled(false);
        return {};
    }
}

void TransactionEditor::showEvent(QShowEvent*) {
    FillData();
}
