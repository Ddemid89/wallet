#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>
#include <QDate>

#include "model_representation.h"

enum class TransactType {
    Income,
    Expense,
    Transfer,
    All,
    IncExp
};

enum class AccountType {
    Debet,
    Credit,
    Deposit,
    Overdraft
};

class Money {
public:
    Money() = default;
    Money(long cop);
    double Double() const;
    long Kopek() const;
    void FromDouble(double val);
    void operator+=(const Money other);
    void operator-=(const Money other);
    QString String() const;
    QString StringAbs() const;
    Money operator-(Money rhs) const;
    Money& operator=(double val);

private:
    long kopek_{0};
};

Money operator""_money(long double val);

class AccountBase;
class Debet;
class PercentBase;
class Credit;
class Deposit;
class OverdraftCard;

class AccVisitorInterface {
public:
    virtual void SetAccount(Debet&) = 0;
    virtual void SetAccount(Credit&) = 0;
    virtual void SetAccount(Deposit&) = 0;
    virtual void SetAccount(OverdraftCard&) = 0;
};

struct SimpleAcc {
    size_t idx;
    QString name;
    Money sum;
    bool consider;
};

class AccountBase {
public:
    AccountBase(SimpleAcc acc);
    AccountBase(const model_representation::AccountRepresentation& repr)
        : idx_(repr.id)
        , name_(repr.name)
        , sum_(repr.balance_kopek)
        , consider_(repr.consider)
        , deleted_(repr.deleted) {}
    virtual void Process(QDate today) = 0;
    virtual Money GetSum() const = 0;
    virtual void operator+=(Money rhs) = 0;
    virtual void operator-=(Money rhs) = 0;
    QString GetName() const;
    virtual ~AccountBase() = default;
    size_t GetIndex() const;
    virtual model_representation::AccountRepresentation GetRepresentation() const = 0;

    virtual void Visit(AccVisitorInterface& widget) = 0;

    void SetDeleted(bool deleted) {
        deleted_ = deleted;
    }

    bool IsDeleted() const {
        return deleted_;
    }
protected:
    size_t idx_;
    QString name_;
    Money sum_;
    bool consider_;
    bool deleted_ = false;
};

class Debet : public AccountBase {
public:
    Debet(SimpleAcc acc);
    Debet(const model_representation::AccountRepresentation& repr)
        : AccountBase(repr) {}

    void Process(QDate today) override;
    Money GetSum() const override;
    void operator+=(Money rhs) override;
    void operator-=(Money rhs) override;
    void Visit(AccVisitorInterface& widget) override;
    model_representation::AccountRepresentation GetRepresentation() const override {
        model_representation::AccountRepresentation res;

        res.id = idx_;
        res.name = name_;
        res.balance_kopek = sum_.Kopek();
        res.consider = consider_;
        res.deleted = deleted_;

        res.type = static_cast<int>(AccountType::Debet);

        return res;
    }
};

class PercentBase : public AccountBase {
public:
    PercentBase(SimpleAcc acc, double percent_rate, int payday);
    PercentBase(const model_representation::AccountRepresentation& repr)
        : AccountBase(repr)
        , percent_rate_(repr.perc_rate)
        , payday_(repr.payday)
        , next_pay_(repr.next_pay) {}
protected:
    ~PercentBase() override = default;
    double percent_rate_;
    int payday_;
    QDate next_pay_;
};

class Credit : public PercentBase {
public:
    Credit(SimpleAcc acc, double percent_rate, int payday, Money payment);
    Credit(const model_representation::AccountRepresentation& repr)
        : PercentBase(repr)
        , payment_(repr.payment) {}
    void Process(QDate today) override;
    Money GetSum() const override;
    void operator+=(Money rhs) override;
    void operator-=(Money rhs) override;
    void Visit(AccVisitorInterface& widget) override;
    model_representation::AccountRepresentation GetRepresentation() const override {
        model_representation::AccountRepresentation res;

        res.id = idx_;
        res.name = name_;
        res.balance_kopek = sum_.Kopek();
        res.consider = consider_;
        res.deleted = deleted_;
        res.perc_rate = percent_rate_;
        res.payday = payday_;
        res.next_pay = next_pay_;
        res.payment = payment_.Kopek();

        res.type = static_cast<int>(AccountType::Credit);

        return res;
    }
private:
    Money payment_;
};

class Deposit : public PercentBase {
public:
    Deposit(SimpleAcc acc, double percent_rate, int payday);
    Deposit(const model_representation::AccountRepresentation& repr)
        : PercentBase(repr) {}
    void Process(QDate today) override;
    Money GetSum() const override;
    void operator+=(Money rhs) override;
    void operator-=(Money rhs) override;
    void Visit(AccVisitorInterface& widget) override;
    model_representation::AccountRepresentation GetRepresentation() const override {
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
};

class OverdraftCard : public PercentBase {
public:
    OverdraftCard(SimpleAcc acc, double percent_rate, int payday, Money overdraft);
    OverdraftCard(const model_representation::AccountRepresentation& repr)
        : PercentBase(repr)
        , overdraft_(repr.overdraft) {}
    void Process(QDate today) override;
    Money GetSum() const override;
    void operator+=(Money rhs) override;
    void operator-=(Money rhs) override;
    void Visit(AccVisitorInterface& widget) override;
    Money OwnSum() const;
    Money Debt() const;
    model_representation::AccountRepresentation GetRepresentation() const override {
        model_representation::AccountRepresentation res;

        res.id = idx_;
        res.name = name_;
        res.balance_kopek = sum_.Kopek();
        res.consider = consider_;
        res.deleted = deleted_;
        res.perc_rate = percent_rate_;
        res.payday = payday_;
        res.next_pay = next_pay_;
        res.overdraft = overdraft_.Kopek();

        res.type = static_cast<int>(AccountType::Overdraft);

        return res;
    }
private:
    Money overdraft_;
};

class Category {
public:
    Category(size_t idx, QString name, bool inc, bool dec);
    Category(const model_representation::CategoryRepresentation& repr, Category* parent)
        : idx_(repr.id)
        , name_(repr.name)
        , inc_(repr.inc)
        , dec_(repr.dec)
        , childs_{}
        , deleted_(repr.deleted)
    {
        parent->AddChild(*this);
    }

    void AddChild(Category& child);
    QString GetName() const;

    bool isInc() const;

    bool isDec() const;

    std::vector<size_t> GetChilds() const {
        return childs_;
    }

    void SetDeleted(bool deleted) {
        deleted_ = deleted;
    }

    bool isDeleted() const {
        return deleted_;
    }

    model_representation::CategoryRepresentation GetRepresentation() const {
        model_representation::CategoryRepresentation res;

        res.name = name_;
        res.inc = inc_;
        res.dec = dec_;
        res.id = idx_;

        if (parent_ != nullptr) {
            res.parent_id = parent_->idx_;
        } else {
            res.parent_id = 0;
        }

        return res;
    }
private:
    void SetInc();
    void SetDec();

    size_t idx_;
    QString name_;
    bool inc_;
    bool dec_;
    std::vector<size_t> childs_;
    Category* parent_ = nullptr;
    bool deleted_ = false;
};

class TransactBase;
class Transaction;
class Transfer;

class TransferVisitorInterface {
public:
    virtual ~TransferVisitorInterface() = default;
    virtual void Visit(Transaction&) = 0;
    virtual void Visit(Transfer&) = 0;
    virtual void Visit(TransactBase&) = 0;
};

class TransactBase{
public:
    TransactBase(size_t idx, size_t acc, QDate date, Money sum);
    TransactBase(model_representation::TransactionRepresentation& trs);
    size_t Index() const;
    size_t AccountFromIdx() const;
    QDate Date() const;
    Money Sum() const;
    virtual void Visit(TransferVisitorInterface& visitor) = 0;
    virtual model_representation::TransactionRepresentation GetRepresentation() const = 0;
    virtual ~TransactBase() = default;
protected:
    size_t idx_;
    size_t account_from_idx_;
    QDate date_;
    Money sum_;
};

class Transaction : public TransactBase {
public:
    Transaction(size_t idx, size_t acc, QDate date, Money sum, size_t cat, bool inc);
    Transaction(model_representation::TransactionRepresentation& trs, bool inc);
    size_t CategoryIdx() const;
    bool IsIncome() const;
    void Visit(TransferVisitorInterface& visitor) override {
        visitor.Visit(*this);
    }
    model_representation::TransactionRepresentation GetRepresentation() const override {
        model_representation::TransactionRepresentation res;

        res.from_id = account_from_idx_;
        res.date = date_;
        res.sum = sum_.Kopek();
        res.to_id = category_idx_;
        res.id = idx_;

        model_representation::TransType type;

        if (inc_) {
            type = model_representation::TransType::Income;
        } else {
            type = model_representation::TransType::Expense;
        }

        res.type = static_cast<int>(type);

        return res;
    }
private:
    size_t category_idx_;
    bool inc_;
};

class Transfer : public TransactBase {
public:
    Transfer(size_t idx, size_t acc, QDate date, Money sum, size_t acc_to);
    Transfer(model_representation::TransactionRepresentation& trs)
        : TransactBase(trs)
        , account_to_idx_(trs.to_id) {}
    size_t AccountToIdx() const;
    void Visit(TransferVisitorInterface& visitor) override {
        visitor.Visit(*this);
    }
    model_representation::TransactionRepresentation GetRepresentation() const override {
        model_representation::TransactionRepresentation res;

        res.from_id = account_from_idx_;
        res.date = date_;
        res.sum = sum_.Kopek();
        res.to_id = account_to_idx_;
        res.id = idx_;

        model_representation::TransType type;

        type = model_representation::TransType::Transfer;

        res.type = static_cast<int>(type);

        return res;
    }
private:
    size_t account_to_idx_;
};

#endif // DOMAIN_H
