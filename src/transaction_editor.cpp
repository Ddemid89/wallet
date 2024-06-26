#include "transaction_editor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

#ifdef Q_OS_WINDOWS
    const auto MONOSPACE_FONT = "Courier";
    const int WIDTH = 108;
    const int LAB_WIDTH = 250;
    const int END_WIDTH = 259;
#else
    const auto MONOSPACE_FONT = "Monospace";
    const int WIDTH = 97;
    const int LAB_WIDTH = 278;
    const int END_WIDTH = 200;
#endif

namespace  {
QString FormatOperation(Wallet& wallet, const Transaction* trns_ptr) {
    QString res = trns_ptr->Date().toString("dd.MM.yy")
                  + "  |  " + wallet.GetAccName(trns_ptr->AccountFromIdx());

    const int first_h_line = 34;
    const int arrow_indent = 3;
    const int arrow_length = 23;
    const int arrow_tail   = 6;
    const int second_h_line_indent = 6;

    const int arrow_start  = first_h_line + arrow_indent;
    const int sum_end      = arrow_start + arrow_length - arrow_tail;

    res += QString(first_h_line - res.size(), ' ');

    res += "|" + QString(arrow_indent, ' ');

    if (trns_ptr->Type() == TransactionType::Income) {
        res += " <";
    } else {
        res += " -";
    }

    QString sum_str = trns_ptr->Sum().StringAbs();

    res += QString(sum_end - res.size() - sum_str.size(), '-');

    if (trns_ptr->Type() == TransactionType::Transfer) {
        res += "[";
    } else {
        res += "(";
    }

    res += sum_str + " руб.";

    if (trns_ptr->Type() == TransactionType::Transfer) {
        res += "]";
    } else {
        res += ")";
    }

    res += QString(arrow_tail - 1, '-');

    if (trns_ptr->Type() == TransactionType::Income) {
        res += "- ";
    } else if (trns_ptr->Type() == TransactionType::Expense) {
        res += "> ";
    } else {
        res += "> ";
    }

    res += QString(second_h_line_indent, ' ') + "|";

    QString to;

    if (trns_ptr->Type() == TransactionType::Income) {
        to = wallet.GetCatName(trns_ptr->ToIdx());
    } else if (trns_ptr->Type() == TransactionType::Expense) {
        to = wallet.GetCatName(trns_ptr->ToIdx());
    } else {
        to = wallet.GetAccName(trns_ptr->ToIdx());
    }

    res += QString(WIDTH - res.size() - to.size(), ' ');

    res += to;

    return res;
}

QString MakeLength(QString txt, size_t len) {
    return QString(len - txt.size(), ' ') + txt;
}

}

TransactionEditor::TransactionEditor(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                            : Widgets(wallet, m_window, parent) {
    QHBoxLayout* date_select_ = new QHBoxLayout;
    QHBoxLayout* type_select_ = new QHBoxLayout;
    QHBoxLayout* btns_layout_ = new QHBoxLayout;

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
    layout_->addLayout(btns_layout_);
    layout_->addWidget(list_);

    setLayout(layout_);

    QVBoxLayout* info_layout = new QVBoxLayout;
    QHBoxLayout* inc_info    = new QHBoxLayout;
    QHBoxLayout* dec_info    = new QHBoxLayout;
    QHBoxLayout* tot_info    = new QHBoxLayout;

    QFont font(MONOSPACE_FONT);

    info_layout->addLayout(inc_info);
    info_layout->addLayout(dec_info);
    info_layout->addLayout(tot_info);

    QLabel* inc_label = new QLabel("Итого доходов: |");
    QLabel* dec_label = new QLabel("Итого расходов: |");
    QLabel* tot_label = new QLabel("Итого: |");

    QLabel* inc_end = new QLabel("|");
    QLabel* dec_end = new QLabel("|");
    QLabel* tot_end = new QLabel("|");

    inc_label->setFixedWidth(LAB_WIDTH);
    dec_label->setFixedWidth(LAB_WIDTH);
    tot_label->setFixedWidth(LAB_WIDTH);

    inc_end->setFixedWidth(END_WIDTH);
    dec_end->setFixedWidth(END_WIDTH);
    tot_end->setFixedWidth(END_WIDTH);

    inc_info->addWidget(inc_label);
    dec_info->addWidget(dec_label);
    tot_info->addWidget(tot_label);

    inc_info->addWidget(inc_data_);
    dec_info->addWidget(dec_data_);
    tot_info->addWidget(tot_data_);

    inc_info->addWidget(inc_end);
    dec_info->addWidget(dec_end);
    tot_info->addWidget(tot_end);

    inc_label->setFont(font);
    dec_label->setFont(font);
    tot_label->setFont(font);

    inc_end->setFont(font);
    dec_end->setFont(font);
    tot_end->setFont(font);

    inc_data_->setFont(font);
    dec_data_->setFont(font);
    tot_data_->setFont(font);

    inc_label->setAlignment(Qt::AlignRight);
    dec_label->setAlignment(Qt::AlignRight);
    tot_label->setAlignment(Qt::AlignRight);

    connect(date_from_, &QDateEdit::dateChanged, [this]{
        date_to_->setMinimumDate(date_from_->date());
    });
    connect(date_to_, &QDateEdit::dateChanged, [this]{
        date_from_->setMaximumDate(date_to_->date());
    });

    list_->setFont(font);

    connect(list_, &QListWidget::itemDoubleClicked, this, &TransactionEditor::Edit);

    connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(FillTargets()));
    connect(show_data, SIGNAL(clicked()), this, SLOT(FillOps()));

    QPushButton* del_btn = new QPushButton("Удалить");
    del_btn->setFixedWidth(120);
    btns_layout_->addWidget(del_btn);

    QPushButton* edit_btn = new QPushButton("Редактировать");
    edit_btn->setFixedWidth(120);
    btns_layout_->addWidget(edit_btn);

    layout_->addLayout(info_layout);

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

    Money inc, dec;

    for (auto& op : ops) {
        if (op->Type() == TransactionType::Income) {
            inc += op->Sum();
        } else if (op->Type() == TransactionType::Expense) {
            dec += op->Sum();
        }
        list_->addItem(FormatOperation(wallet_, op) /*+ " (id: " + QString::number(op->Index()) + ")"*/ );
        trns_idxs_.push_back(op->Index());
    }

    const size_t LENGTH = 19;

    inc_data_->setText(MakeLength(inc.StringAbs(), LENGTH));
    dec_data_->setText(MakeLength(dec.StringAbs(), LENGTH));
    tot_data_->setText(MakeLength((inc - dec).String(), LENGTH));
}

void TransactionEditor::Edit() {
    int idx = list_->currentRow();

    if (idx == -1) {
        return;
    }

    const Transaction* op = wallet_.FindTransact(trns_idxs_.at(idx));

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
