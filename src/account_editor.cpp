#include "account_editor.h"

#include <QMessageBox>
#include <QCloseEvent>

void AccountEditor::SetAccount(Debet &) {
    setWindowTitle("Дебетовый счет");
    type_ = AccountType::Debet;
    DrawDepOvr();
    setFixedHeight(200);
}

void AccountEditor::SetAccount(Credit& acc) {
    setWindowTitle("Кредит");
    type_ = AccountType::Credit;
    sum_lab_->setText("Остаток долга:");
    DrawPayment();
    payment_->setValue(acc.GetPayment().Double());
    payment_lab_->setText("Ежемесячный платеж:");
    DrawPercentDay();
    perc_->setValue(acc.GetPercentRate());
    day_->setValue(acc.GetPayDay());
    setFixedHeight(250);
}

void AccountEditor::SetAccount(Deposit& acc) {
    setWindowTitle("Вклад");
    type_ = AccountType::Deposit;
    DrawPercentDay();
    perc_->setValue(acc.GetPercentRate());
    day_->setValue(acc.GetPayDay());
    setFixedHeight(250);
    DrawDepOvr();
}

void AccountEditor::SetAccount(OverdraftCard& acc) {
    setWindowTitle("Карта с овердрафтом");
    type_ = AccountType::Overdraft;
    DrawPayment();
    payment_lab_->setText("Размер овердрафта:");
    payment_->setValue(acc.GetOverdraft().Double());
    DrawPercentDay();
    perc_->setValue(acc.GetPercentRate());
    day_->setValue(acc.GetPayDay());
    setFixedHeight(280);
    DrawDepOvr();
}

void AccountEditor::closeEvent(QCloseEvent* event) {
    if (NoChanged()) {
        delete this;
        return;
    }

    QMessageBox::StandardButton save
        = QMessageBox::question(this, "Сохранение изменений",
                                "Сохранить внесенные изменения?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (save == QMessageBox::No) {
        delete this;
        return;
    } else if (save == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    SaveAccount(event);
}

void AccountEditor::DepClicked() {
    bool dep = dep_->isChecked();
    bool ovr = ovr_->isChecked();

    if (dep && ovr) {
        ovr_->setChecked(false);
    }
}

void AccountEditor::OvrClicked() {
    bool dep = dep_->isChecked();
    bool ovr = ovr_->isChecked();

    if (dep && ovr) {
        dep_->setChecked(false);
    }
}

void AccountEditor::OnDone() {
    if (NoChanged()) {
        delete this;
        return;
    }
    SaveAccount();
}

void AccountEditor::DrawPercentDay() {
    perc_ = new QDoubleSpinBox;
    day_  = new QSpinBox;

    perc_->setMinimum(0.01);
    perc_->setMaximum(200.);
    perc_->setSuffix(" %");

    day_->setMaximum(31);
    day_->setMinimum(1);

    layout_->addRow("Процентная ставка:", perc_);
    layout_->addRow("День платежа:", day_);
}

void AccountEditor::DrawPayment() {
    payment_ = new QDoubleSpinBox;
    payment_->setMinimum(0.01);
    payment_->setMaximum(1000000);
    payment_->setSuffix(" руб.");

    payment_lab_ = new QLabel;

    layout_->addRow(payment_lab_, payment_);
}

void AccountEditor::DrawDepOvr() {
    dep_ = new QCheckBox("Проценты на остаток");
    ovr_ = new QCheckBox("Овердрафт");

    dep_->setChecked(type_ == AccountType::Deposit);
    ovr_->setChecked(type_ == AccountType::Overdraft);

    layout_->addRow(dep_);
    layout_->addRow(ovr_);
    connect(dep_, &QCheckBox::clicked, this, &AccountEditor::DepClicked);
    connect(ovr_, &QCheckBox::clicked, this, &AccountEditor::OvrClicked);
}

void AccountEditor::ChangeData() {
    AccountBase* acc = wallet_.GetOneAccount(id_);
    AccountChanger ac{*this};
    acc->Visit(ac);
}

void AccountEditor::ChangeTypeAndData() {
    AccAdder adder;
    adder.name = name_->text().trimmed();
    adder.balance = sum_->value();
    adder.consider = consider_->isChecked();
    if (payment_) {
        adder.overdraft = payment_->value();
        adder.payment   = adder.overdraft;
    } else {
        adder.overdraft = adder.payment = 0.01;
    }

    if (day_) {
        adder.payday = day_->value();
    } else {
        adder.payday = 5;
    }

    if (perc_) {
        adder.perc_rate = perc_->value();
    } else {
        adder.perc_rate = 0.01;
    }

    adder.type = GetNewType();
    wallet_.ChangeAccountType(adder, id_);
}

void AccountEditor::SaveAccount(QCloseEvent* event) {
    if (!Correct()) {
        if (event) {
            event->ignore();
        }
        return;
    }

    AccountType new_type = GetNewType();

    if (type_ != new_type) {
        const QString names[] = {"ДЕБЕТОВЫМ СЧЕТОМ", "ОШИБКА!!!!!", "ВКЛАДОМ", "КАРТОЙ С ОВЕРДРАФТОМ"};
        int was = static_cast<int>(type_);
        int now = static_cast<int>(new_type);

        const QString txt = "Сейчас счет является " + names[was] + ". Хотите сделать его " + names[now] + "?";

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Измениние типа счета", txt, QMessageBox::Yes | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) {
            if (event) {
                event->ignore();
            }
            return;
        }
        ChangeTypeAndData();
        emit Changed();

        if (new_type == AccountType::Deposit || new_type == AccountType::Overdraft) {
            AccountBase* acc = wallet_.GetOneAccount(id_);
            AccountEditor* ae = new AccountEditor(wallet_, id_);

            acc->Visit(*ae);

            ae->show();
        }

        delete this;
        return;
    }
    ChangeData();
    emit Changed();
    delete this;
}

AccountType AccountEditor::GetNewType() {
    if (type_ == AccountType::Credit) {
        return type_;
    }

    bool dep = dep_->isChecked();
    bool ovr = ovr_->isChecked();

    if (!dep && !ovr) {
        return AccountType::Debet;
    }

    if (dep) {
        return AccountType::Deposit;
    }

    return AccountType::Overdraft;
}

bool AccountEditor::NoChanged() {
    AccountBase* acc = wallet_.GetOneAccount(id_);

    QString old_name = acc->GetName();
    QString new_name = name_->text().trimmed();

    Money old_sum    = acc->GetSum();
    Money new_sum    = sum_->value();

    bool old_conside = acc->IsConsider();
    bool new_conside = consider_->isChecked();

    bool payday  = true;
    bool perc    = true;
    bool payment = true;

    if (type_ == AccountType::Credit) {
        auto cred = dynamic_cast<Credit*>(acc);
        payday  = day_->value()     == cred->GetPayDay();
        perc    = perc_->value()    == cred->GetPercentRate();
        payment = Money(payment_->value()) == cred->GetPayment();
    } else if (type_ == AccountType::Deposit) {
        auto depos = dynamic_cast<Deposit*>(acc);
        payday = day_->value()  == depos->GetPayDay();
        perc   = perc_->value() == depos->GetPercentRate();
    } else if (type_ == AccountType::Overdraft) {
        auto over = dynamic_cast<OverdraftCard*>(acc);
        payment   = Money(payment_->value()) == over->GetOverdraft();
        perc      = perc_->value() == over->GetPercentRate();
        payday    = day_->value()  == over->GetPayDay();
    }

    return type_ == GetNewType() && old_name == new_name && old_sum == new_sum
           && old_conside == new_conside && payday && perc && payment;
}

bool AccountEditor::Correct() {
    if (name_->text().trimmed() == "") {
        QMessageBox::warning(this, "Неверные данные", "Название счета не может быть пустым или состоять только из пробелов.");
        return false;
    }
    return true;
}

AccountEditor::AccountChanger::AccountChanger(AccountEditor& editor) : editor_{editor} { }

void AccountEditor::AccountChanger::SetAccount(Debet& acc) {
    FillCommon(acc);
}

void AccountEditor::AccountChanger::SetAccount(Credit& acc) {
    FillCommon(acc);
    FillPercDay(acc);
    acc.SetPayment(Money{editor_.payment_->value()});
}

void AccountEditor::AccountChanger::SetAccount(Deposit& acc) {
    FillCommon(acc);
    FillPercDay(acc);
}

void AccountEditor::AccountChanger::SetAccount(OverdraftCard& acc) {
    FillCommon(acc);
    FillPercDay(acc);
    acc.SetOverdraft(Money{editor_.payment_->value()});
}

void AccountEditor::AccountChanger::FillCommon(AccountBase &acc) {
    acc.SetName(editor_.name_->text().trimmed());
    acc.SetSum(Money{editor_.sum_->value()});
    acc.SetConsider(editor_.consider_->isChecked());
}

void AccountEditor::AccountChanger::FillPercDay(PercentBase& acc) {
    acc.SetPayDay(editor_.day_->value());
    acc.SetPercentRate(editor_.perc_->value());
}
