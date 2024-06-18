#include "add_account_window.h"
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include "account_editor.h"

AddAccountWindow::AddAccountWindow(Wallet& wallet, MainWindow& m_window, QWidget* parent)
                                        : Widgets(wallet, m_window, parent){
    QLabel* lab = new QLabel("Все счета:");
    QFormLayout* layout = new QFormLayout;
    QLabel* lab2 = new QLabel("Новый счет:");
    QFormLayout* additional = new QFormLayout;
    QPushButton* button = new QPushButton("Создать счет");
    QHBoxLayout* del_edit = new QHBoxLayout;

    lab2->setFrameStyle(2);

    type_->addItems({"Дебетовый", "Накопительный", "Кредит", "С овердрафтом"});

    start_sum_->setMaximum(1000000000);
    start_sum_->setMinimum(-1000000000);
    start_sum_->setSuffix(" руб.");

    consider_->setLayoutDirection(Qt::LayoutDirection::RightToLeft);

    overdraft_->setSuffix(" руб.");
    overdraft_->setMinimum(0.01);
    overdraft_->setMaximum(1000000);

    payment_->setMinimum(0.01);
    payment_->setMaximum(1000000);
    payment_->setSuffix(" руб.");

    percent_rate_->setSuffix(" %");
    percent_rate_->setMinimum(0.1);
    percent_rate_->setMaximum(100);

    date_->setMinimum(1);
    date_->setMaximum(31);

    QPushButton* edit = new QPushButton("Редактировать");

    del_edit->addSpacing(100);
    del_edit->addWidget(edit);
    del_edit->addSpacing(200);
    del_edit->addWidget(del_);
    del_edit->addSpacing(100);

    connect(type_, &QComboBox::currentIndexChanged, this, &AddAccountWindow::select);
    connect(button, SIGNAL(clicked()), this, SLOT(submit()));
    connect(accs_, &QListWidget::currentItemChanged, this, &AddAccountWindow::SelectAccount);
    connect(del_, SIGNAL(clicked()), SLOT(DeleteRestore()));
    connect(edit, SIGNAL(clicked()), SLOT(Edit()));
    connect(accs_, &QListWidget::itemDoubleClicked, this, &AddAccountWindow::Edit);

    layout_->addWidget(back_);
    layout_->addWidget(lab);
    layout_->addWidget(accs_);
    layout_->addLayout(del_edit);
    layout_->addWidget(lab2);

    layout->addRow(new QLabel("Название счета: "), name_);
    layout->addRow(new QLabel("Вид счета: "), type_);
    layout->addRow(new QLabel("Сумма на данный момент: "), start_sum_);
    layout->addWidget(consider_);

    additional->addRow(new QLabel("Ежемесячный платеж: "), payment_);
    additional->addRow(new QLabel("Размер овердрафта: "), overdraft_);
    additional->addRow(new QLabel("Процентная ставка: "), percent_rate_);
    additional->addRow(new QLabel("Дата начисления/списания процентов: "), date_);

    layout_->addLayout(layout);
    layout_->addLayout(additional);

    layout_->addWidget(button);

    FillData();

}

void AddAccountWindow::select() {
    int type = type_->currentIndex();

    percent_rate_->setEnabled(type > 0);
    date_->setEnabled(type > 0);
    overdraft_->setEnabled(type == 3);
    payment_->setEnabled(type == 2);
}

void AddAccountWindow::submit() {
    QString name = name_->text();
    name = name.trimmed();

    if (name == "") {
        name_->setText("Введите название счета");
        return;
    }

    int type = type_->currentIndex();

    AccAdder adder;
    adder.balance.FromDouble(start_sum_->value());
    adder.name = name;
    adder.perc_rate = percent_rate_->value();
    adder.payday = date_->value();
    adder.overdraft.FromDouble(overdraft_->value());
    adder.consider = consider_->isChecked();
    adder.payment.FromDouble(payment_->value());

    switch (type) {
    case 0:
        adder.type = AccountType::Debet;
        break;
    case 1:
        adder.type = AccountType::Deposit;
        break;
    case 2:
        adder.type = AccountType::Credit;
        break;
    case 3:
        adder.type = AccountType::Overdraft;
        break;
    }

    accs_->addItem(name);
    wallet_.AddAccount(std::move(adder));

    emit(m_window_.show_status("Счет \"" + name + "\" добавлен!"));

    name_->clear();
}

void AddAccountWindow::DeleteRestore() {
    int row = accs_->currentRow();
    if (row == -1) {
        return;
    }
    size_t acc_id = accs_index_.at(row);

    bool deleted = wallet_.IsAccountDeleted(acc_id);
    wallet_.SetAccountDeleted(acc_id, !deleted);
    FillAccs();
}

void AddAccountWindow::Edit() {
    int row = accs_->currentRow();
    if (row == -1) {
        return;
    }
    size_t acc_id = accs_index_.at(row);

    AccountBase* acc = wallet_.GetOneAccount(acc_id);
    AccountEditor* ae = new AccountEditor(wallet_, acc_id);

    acc->Visit(*ae);

    connect(ae, &AccountEditor::Changed, this, &AddAccountWindow::FillAccs);

    ae->show();
}

void AddAccountWindow::SelectAccount() {
    int row = accs_->currentRow();
    if (row == -1) {
        return;
    }
    size_t acc_id = accs_index_.at(row);

    if (wallet_.IsAccountDeleted(acc_id)) {
        del_->setText("Восстановить");
    } else {
        del_->setText("Удалить");
    }
}



void AddAccountWindow::FillData() {
    consider_->setChecked(true);
    FillAccs();
    type_->setCurrentIndex(0);
    start_sum_->setValue(0);
    name_->setText("");

    percent_rate_->setEnabled(false);
    date_->setEnabled(false);
    overdraft_->setEnabled(false);
    payment_->setEnabled(false);

    percent_rate_->setValue(0.1);
    date_->setValue(1);
    overdraft_->setValue(0.01);
    payment_->setValue(0.01);
}

void AddAccountWindow::FillAccs() {
    const auto& accs = wallet_.GetAccounts();

    QVector<AccountBase*> deleted;

    accs_->clear();
    accs_index_.clear();

    for (auto& acc : accs) {
        if (!acc->IsDeleted()) {
            accs_index_.push_back(acc->GetIndex());
            accs_->addItem(acc->GetName() + " (" + acc->GetSum().String() + " руб.)");
        } else {
            deleted.push_back(acc.get());
        }
    }

    for (auto acc : deleted) {
        accs_index_.push_back(acc->GetIndex());
        accs_->addItem(acc->GetName() + " (удален)");
    }
}

void AddAccountWindow::showEvent(QShowEvent*) {
    FillData();
}
