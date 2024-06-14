#include "transaction_editor.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>

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

    connect(list_, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(Edit(QListWidgetItem*)));

    connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(FillTargets()));
    connect(show_data, SIGNAL(clicked()), this, SLOT(FillOps()));

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
    trns_ptrs_.clear();

    QDate from = date_from_->date();
    QDate to = date_to_->date();

    TransactType type = TransactType::All;

    switch (type_->currentIndex()) {
    case 0:
        type = TransactType::All;
        break;
    case 1:
        type = TransactType::Income;
        break;
    case 2:
        type = TransactType::Expense;
        break;
    case 3:
        type = TransactType::Transfer;
        break;
    }

    size_t acc = accs_idxs_.at(acc_->currentIndex());
    size_t cat = 0;

    if (type != TransactType::Transfer) {
        cat = cats_idxs_.at(target_->currentIndex());
    }

    auto ops = wallet_.GetTransactFiltred(from, to, type, acc, cat);

    detail::TransactInfo tr(wallet_);

    for (auto& op : ops) {
        op->Visit(tr);
        list_->addItem(tr.GetResult());
        trns_ptrs_.push_back(op);
    }
}

void TransactionEditor::Edit(QListWidgetItem*) {
    size_t idx = list_->currentRow();
    TransactBase* op = trns_ptrs_.at(idx);


    qDebug() << op->Sum().String();
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
