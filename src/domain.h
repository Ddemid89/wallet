#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>
#include <QDate>

#include "model_representation.h"

enum class TransactShowType {
    Income,
    Expense,
    Transfer,
    All,
    IncExp
};

enum class TransactionType{
    Income,
    Expense,
    Transfer
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
    Money(double val);
    Money(qint64 val);
    Money(int val);
    double Double() const;
    long Kopek() const;
    void FromDouble(double val);
    void operator+=(const Money other);
    void operator-=(const Money other);
    bool operator==(const Money other) const;
    QString String() const;
    QString StringAbs() const;
    Money operator-(Money rhs) const;
    Money& operator=(double val);

private:
    long kopek_{0};
};

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

    void SetName(const QString& name) {
        name_ = name;
    }

    void SetSum(Money money) {
        sum_ = money;
    }

    void SetConsider(bool consider) {
        consider_ = consider;
    }

    void SetDeleted(bool deleted) {
        deleted_ = deleted;
    }

    bool IsDeleted() const {
        return deleted_;
    }

    bool IsConsider() const {
        return consider_;
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
    double GetPercentRate() const {
        return percent_rate_;
    }

    void SetPercentRate(double val) {
        percent_rate_ = val;
    }

    int GetPayDay() const {
        return payday_;
    }

    void SetPayDay(int val) {
        payday_ = val;
    }

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
    Money GetPayment() const;
    void SetPayment(Money money);
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
    Money GetOverdraft() const;
    void SetOverdraft(Money money);
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
};

class Transaction_DEL{
public:
    Transaction_DEL(size_t idx, size_t from, size_t to, TransactionType type, QDate date, Money sum);
    Transaction_DEL(model_representation::TransactionRepresentation& trs);
    size_t Index() const;
    size_t AccountFromIdx() const;
    size_t ToIdx() const;
    QDate Date() const;
    Money Sum() const;
    TransactionType Type() const;
    bool Income();
    model_representation::TransactionRepresentation GetRepresentation() const;
    bool operator<(const Transaction_DEL& other) const;
    void Swap(Transaction_DEL& other);
protected:
    QDate date_;
    size_t account_from_idx_;
    size_t idx_;
    Money sum_;
    size_t to_idx_;
    TransactionType type_;
};

#endif // DOMAIN_H
