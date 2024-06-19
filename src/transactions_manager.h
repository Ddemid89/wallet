#ifndef TRANSACTIONSMANAGER_H
#define TRANSACTIONSMANAGER_H

#include <QDate>
#include <QString>
#include "domain.h"
#include <set>
#include <map>

namespace transactions_manager {

class TransactionMatcher{
public:
    TransactionMatcher(TransactShowType type) {
        if (type == TransactShowType::IncExp) {
            trf_ = false;
        } else if (type == TransactShowType::Expense) {
            inc_ = trf_ = false;
        } else if (type == TransactShowType::Income) {
            dec_ = trf_ = false;
        } else if (type == TransactShowType::Transfer) {
            inc_ = dec_ = false;
        }
    }

    bool Check(TransactionType type) {
        if (type == TransactionType::Income) {
            return inc_;
        } else if (type == TransactionType::Expense) {
            return dec_;
        }
        return trf_;
    }
private:
    bool inc_ = true;
    bool dec_ = true;
    bool trf_ = true;
};

struct TransactionAdder{
    int from_idx;
    int to_idx;
    QDate date;
    Money sum;
    TransactionType type;
    bool IsValid() const {
        return type != TransactionType::Transfer || (type == TransactionType::Transfer && from_idx != to_idx);
    }
};


struct Date {
    Date() = default;
    Date(QDate date) {
        year = date.year();
        month = date.month();
    }
    int year;
    int month;

    bool operator<(Date other) const;
    bool operator<=(Date other) const;

    QString String() const;
};

class LoaderInterface {
public:
    LoaderInterface() = default;
    virtual model_representation::TransactionsPart Load(Date) = 0;
    virtual void Save(Date, model_representation::TransactionsPart&&) = 0;
    virtual ~LoaderInterface() = default;
};

class TransactionsManager {
public:
    TransactionsManager(LoaderInterface&);
    ~TransactionsManager();

    void AddTransaction(TransactionAdder& trs);

    void AddTransaction(TransactionAdder& trs, size_t idx);

    QVector<const Transaction_DEL*> GetTransacts(TransactShowType type = TransactShowType::All,
                                        size_t number = 10, bool late_to_early = true);

    QVector<const Transaction_DEL*> GetTransactFiltred (QDate from, QDate to, TransactShowType type);

    void Save() {
        auto parts = GetRepresentations();

        for (auto& [date, trs] : parts) {
            model_representation::TransactionsPart part;
            part.id = transact_id_;
            part.trs = std::move(trs);
            loader_.Save(date, std::move(part));
        }
    }

    const Transaction_DEL* FindTransact(size_t idx);
    void DeleteTransact(size_t idx);

private:
    void Load(QDate date) {
        if (loaded_.count(date) == 0) {
            auto trs = loader_.Load(date);
            Restore(std::move(trs));
            loaded_.insert(date);
        }
    }

    void Load(QDate from, QDate to) {
        while(Date{from} <= Date{to}) {
            Load(from);
            from = from.addMonths(1);
        }
    }

    void Restore(model_representation::TransactionsPart&& part) {
        for (auto& trs : part.trs) {
            Transaction_DEL new_trs{trs};

            auto pair = transacts_.insert(new_trs);
            trns_idx_[trs.id] = &(*pair.first);
        }
        transact_id_ = part.id;
    }

    std::map<Date, QVector<model_representation::TransactionRepresentation>>
    GetRepresentations() {
        std::map<Date, QVector<model_representation::TransactionRepresentation>> result;

        for (auto& trs : transacts_) {
            auto repr = trs.GetRepresentation();
            result[repr.date].push_back(std::move(repr));
        }

        return result;
    }

    template<class It>
    QVector<const Transaction_DEL*> GetTransacts(It begin, It end, size_t number, TransactShowType type) {
        QVector<const Transaction_DEL*> result;

        TransactionMatcher matcher(type);

        while(number-- && begin != end) {
            if (matcher.Check(begin->Type())) {
                result.push_back(&(*begin));
            }
            ++begin;
        }

        return result;
    }

    LoaderInterface& loader_;

    std::set<Date> loaded_;

    std::unordered_map<size_t, const Transaction_DEL*> trns_idx_;
    std::set<Transaction_DEL> transacts_;

    size_t GetNewId() {
        return transact_id_++;
    }
    size_t transact_id_ = 0;
};
} // namespace transactions_manager

#endif // TRANSACTIONSMANAGER_H
