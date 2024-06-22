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

void Debet::operator-=(Money rhs) {
    sum_ -= rhs;
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

Money Credit::GetPayment() const {
    return payment_;
}

void Credit::SetPayment(Money money) {
    payment_ = money;
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

model_representation::AccountRepresentation Deposit::GetRepresentation() const {
    model_representation::AccountRepresentation res;

    res.id = idx_;
    res.name = name_;
    res.balance_kopek = sum_.Kopek();
    res.consider = consider_;
    res.deleted = deleted_;
    res.perc_rate = percent_rate_;
    res.payday = payday_;
    res.next_pay = next_pay_;

    res.type = static_cast<int>(AccountType::Deposit);

    return res;
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

void OverdraftCard::operator-=(Money rhs) {
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

Money OverdraftCard::GetOverdraft() const {
    return overdraft_;
}

void OverdraftCard::SetOverdraft(Money money) {
    overdraft_ = money;
}

Category::Category(size_t idx, QString name, bool inc, bool dec)
    : idx_(idx), name_(std::move(name)), inc_dec_(inc, dec) {
}

Category::Category(const model_representation::CategoryRepresentation &repr, Category *parent)
    : idx_(repr.id)
    , name_(repr.name)
    , inc_dec_{static_cast<CategoryType>(repr.inc_dec)}
    , childs_{}
{
    parent->AddChild(*this);
}

void Category::AddChild(Category& child) {
    child.parent_ = this;
    childs_.push_back(child.idx_);
    if (child.isInc()) {
        SetInc();
    }
    if (child.isDec()) {
        SetDec();
    }
}

QString Category::GetName() const {
    return name_;
}

bool Category::isInc() const {
    return inc_dec_.IsInc();
}

bool Category::isDec() const {
    return inc_dec_.IsDec();
}

CategoryType Category::GetType() const {
    return inc_dec_;
}

std::vector<size_t> Category::GetChilds() const {
    return childs_;
}

size_t Category::GetId() const {
    return idx_;
}

size_t Category::GetParentId() const {
    if (!parent_) {
        throw std::logic_error("Parent == nullptr in Category::GetParentId");
    }
    return parent_->idx_;
}

void Category::EditCategory(QString new_name, Category *new_parent, bool inc, bool dec) {
    name_    = new_name;
    parent_  = new_parent;
    inc_dec_ = CategoryTypeClass(inc, dec);

    if (new_parent && inc) {
        new_parent->SetInc();
    }

    if (new_parent && dec) {
        new_parent->SetDec();
    }
}

model_representation::CategoryRepresentation Category::GetRepresentation() const {
    model_representation::CategoryRepresentation res;

    res.name = name_;
    res.inc_dec = static_cast<quint8>(inc_dec_);
    res.id = idx_;

    if (parent_ != nullptr) {
        res.parent_id = parent_->idx_;
    } else {
        res.parent_id = 0;
    }

    return res;
}

void Category::SetInc() {
    if (inc_dec_ == CategoryType::Dec) {
        inc_dec_ = CategoryType::Both;
    }
    inc_dec_ = CategoryType::Both;
    if (parent_ != nullptr) {
        parent_->SetInc();
    }
}

void Category::SetDec() {
    if (inc_dec_ == CategoryType::Inc) {
        inc_dec_ = CategoryType::Both;
    }
    if (parent_ != nullptr) {
        parent_->SetDec();
    }
}

Transaction::Transaction(size_t idx, size_t from, size_t to, TransactionType trs_type, QDate date, Money sum, QString description)
    : date_(date)
    , account_from_idx_(from)
    , idx_(idx)
    , sum_(sum)
    , to_idx_(to)
    , type_(trs_type)
    , description_(description)
{}

Transaction::Transaction(model_representation::TransactionRepresentation& trs)
    : date_(trs.date)
    , account_from_idx_(trs.from_id)
    , idx_(trs.id)
    , sum_(trs.sum)
    , to_idx_(trs.to_id)
    , type_(static_cast<TransactionType>(trs.type))
    , description_(trs.description)
{}


size_t Transaction::Index() const {
    return idx_;
}

size_t Transaction::AccountFromIdx() const {
    return account_from_idx_;
}

size_t Transaction::ToIdx() const {
    return to_idx_;
}

QDate Transaction::Date() const {
    return date_;
}

Money Transaction::Sum() const {
    return sum_;
}

TransactionType Transaction::Type() const {
    return type_;
}

model_representation::TransactionRepresentation Transaction::GetRepresentation() const {
    model_representation::TransactionRepresentation res;
    res.date        = date_;
    res.from_id     = account_from_idx_;
    res.id          = idx_;
    res.sum         = sum_.Kopek();
    res.to_id       = to_idx_;
    res.type        = static_cast<int>(type_);
    res.description = description_;
    return res;
}

QString Transaction::GetDescription() const {
    return description_;
}

void Transaction::SetDescription(QString new_discription) {
    description_ = new_discription;
}

bool Transaction::operator<(const Transaction& other) const {
    return date_ < other.date_
           || (date_ == other.date_ && idx_ < other.idx_);
}

void Transaction::Swap(Transaction& other) {
    std::swap(date_,             other.date_);
    std::swap(account_from_idx_, other.account_from_idx_);
    std::swap(idx_,              other.idx_);
    std::swap(sum_,              other.sum_);
    std::swap(to_idx_,           other.to_idx_);
    std::swap(type_,             other.type_);
}

Money::Money(long cop) {
    kopek_ = cop;
}

Money::Money(double cop) {
    kopek_ = cop * 100;
}

Money::Money(qint64 val) {
    kopek_ = val;
}

Money::Money(int val) {
    kopek_ = val;
}

double Money::Double() const {
    return kopek_ / 100 + static_cast<double>(kopek_ % 100) / 100;
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

bool Money::operator==(const Money other) const {
    return kopek_ == other.kopek_;
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



CategoryTypeClass::CategoryTypeClass() {
    type_ = CategoryType::Both;
}

CategoryTypeClass::CategoryTypeClass(bool inc, bool dec) {
    Q_ASSERT(inc || dec);
    if (inc) {
        if (dec) {
            type_ = CategoryType::Both;
        } else {
            type_ = CategoryType::Inc;
        }
    } else {
        type_ = CategoryType::Dec;
    }
}

CategoryTypeClass::operator int() const {
    return static_cast<int>(type_);
}

CategoryTypeClass::operator quint8() const {
    return static_cast<quint8>(type_);
}

CategoryTypeClass::operator CategoryType() const {
    return type_;
}

bool CategoryTypeClass::IsInc() const {
    return type_ == CategoryType::Both || type_ == CategoryType::Inc;
}

bool CategoryTypeClass::IsDec() const {
    return type_ == CategoryType::Both || type_ == CategoryType::Dec;
}
