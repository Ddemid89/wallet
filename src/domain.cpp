#include "domain.h"

AccountBase::AccountBase(SimpleAcc acc)
    : idx_(acc.idx), name_(std::move(acc.name)), sum_(acc.sum), consider_(acc.consider) {}

QString AccountBase::GetName() const {
    return name_;
}

size_t AccountBase::GetIndex() const {
    return idx_;
}

Debet::Debet(SimpleAcc acc) : AccountBase(std::move(acc)) {}

void Debet::Process([[maybe_unused]]QDate today) {

}

Money Debet::GetSum() const {
    return sum_;
}

void Debet::operator+=(Money rhs) {
    sum_ += rhs;
}

void Debet::operator-=([[maybe_unused]]Money rhs) {

}

void Debet::Visit(AccVisitorInterface& widget) {
    widget.SetAccount(*this);
}

PercentBase::PercentBase(SimpleAcc acc, double percent_rate, int payday)
    : AccountBase(std::move(acc))
    , percent_rate_(percent_rate)
    , payday_(payday) {}

Credit::Credit(SimpleAcc acc, double percent_rate, int payday, Money payment)
    : PercentBase(std::move(acc), percent_rate, payday), payment_(payment){}

void Credit::Process([[maybe_unused]]QDate today) {

}

Money Credit::GetSum() const {
    return Money{0l};
}

void Credit::operator+=([[maybe_unused]]Money rhs) {

}

void Credit::operator-=([[maybe_unused]]Money rhs) {

}

void Credit::Visit(AccVisitorInterface& widget) {
    widget.SetAccount(*this);
}

Deposit::Deposit(SimpleAcc acc, double percent_rate, int payday)
    : PercentBase(std::move(acc), percent_rate, payday){}

void Deposit::Process([[maybe_unused]]QDate today) {

}

Money Deposit::GetSum() const {
    return sum_;
}

void Deposit::operator+=([[maybe_unused]]Money rhs) {

}

void Deposit::operator-=([[maybe_unused]]Money rhs) {

}

void Deposit::Visit(AccVisitorInterface& widget) {
    widget.SetAccount(*this);
}

OverdraftCard::OverdraftCard(SimpleAcc acc, double percent_rate, int payday, Money overdraft)
    : PercentBase(std::move(acc), percent_rate, payday)
    , overdraft_(overdraft) {}

void OverdraftCard::Process([[maybe_unused]]QDate today) {

}

Money OverdraftCard::GetSum() const {
    return sum_;
}

void OverdraftCard::operator+=(Money rhs) {
    sum_ += rhs;
}

void OverdraftCard::operator-=([[maybe_unused]]Money rhs) {
    sum_ -= rhs;
}

void OverdraftCard::Visit(AccVisitorInterface& widget) {
    widget.SetAccount(*this);
}

Money OverdraftCard::OwnSum() const {
    auto dif = sum_ - overdraft_;
    if (dif.Kopek() > 0) {
        return dif;
    }
    return Money{0};
}

Money OverdraftCard::Debt() const {
    auto dif = overdraft_ - sum_;
    if (dif.Kopek() > 0) {
        return dif;
    }
    return Money{0};
}

Category::Category(size_t idx, QString name, bool inc, bool dec)
    : idx_(idx), name_(std::move(name)), inc_(inc), dec_(dec) {}

void Category::AddChild(Category& child) {
    child.parent_ = this;
    childs_.push_back(child.idx_);
    if (child.inc_) {
        SetInc();
    }
    if (child.dec_) {
        SetDec();
    }
}

QString Category::GetName() const {
    return name_;
}

bool Category::isInc() const {
    return inc_;
}

bool Category::isDec() const {
    return dec_;
}

void Category::SetInc() {
    inc_ = true;
    if (parent_ != nullptr) {
        parent_->SetInc();
    }
}

void Category::SetDec() {
    dec_ = true;
    if (parent_ != nullptr) {
        parent_->SetDec();
    }
}

TransactBase::TransactBase(size_t idx, size_t acc, QDate date, Money sum)
    : idx_(idx)
    , account_from_idx_(acc)
    , date_(date)
    , sum_(sum) {}

TransactBase::TransactBase(model_representation::TransactionRepresentation& trs)
    : idx_(trs.id)
    , account_from_idx_(trs.from_id)
    , date_(trs.date)
    , sum_(trs.sum) {}

size_t TransactBase::Index() const {
    return idx_;
}

size_t TransactBase::AccountFromIdx() const {
    return account_from_idx_;
}

QDate TransactBase::Date() const {
    return date_;
}

Money TransactBase::Sum() const {
    return sum_;
}

Transaction::Transaction(size_t idx, size_t acc, QDate date, Money sum, size_t cat, bool inc)
    : TransactBase(idx, acc, date, sum)
    , category_idx_(cat), inc_(inc) {}

Transaction::Transaction(model_representation::TransactionRepresentation& trs, bool inc)
    : TransactBase(trs)
    , category_idx_(trs.to_id)
    , inc_(inc) {}

size_t Transaction::CategoryIdx() const {
    return category_idx_;
}

bool Transaction::IsIncome() const {
    return inc_;
}

Transfer::Transfer(size_t idx, size_t acc, QDate date, Money sum, size_t acc_to)
    : TransactBase(idx, acc, date, sum)
    , account_to_idx_(acc_to) {}

size_t Transfer::AccountToIdx() const {
    return account_to_idx_;
}

Money::Money(long cop) {
    kopek_ = cop;
}

double Money::Double() const {
    return kopek_ / 100 + (kopek_ % 100) / 100;
}

long Money::Kopek() const {
    return kopek_;
}

void Money::FromDouble(double val) {
    kopek_ = val * 100;
}

void Money::operator+=(const Money other) {
    kopek_ += other.kopek_;
}

void Money::operator-=(const Money other) {
    kopek_ -= other.kopek_;
}

QString Money::String() const {
    QString res;

    long kop = kopek_;

    if (kop < 0) {
        kop = -kop;
        res = "-";
    }

    if (kopek_ % 100 < 10) {
        res += QString::number(kop / 100) + ".0" + QString::number(kop % 100);
    } else {
        res += QString::number(kop / 100) + "." + QString::number(kop % 100);
    }

    return res;
}

QString Money::StringAbs() const {
    QString res;

    long kop = kopek_;

    if (kop < 0) {
        kop = -kop;
    }

    if (kopek_ % 100 < 10) {
        res += QString::number(kop / 100) + ".0" + QString::number(kop % 100);
    } else {
        res += QString::number(kop / 100) + "." + QString::number(kop % 100);
    }

    return res;
}

Money Money::operator-(Money rhs) const {
    Money res;
    res.kopek_ = kopek_ - rhs.kopek_;
    return res;
}

Money& Money::operator=(double val) {
    kopek_ = val * 100;
    return *this;
}


