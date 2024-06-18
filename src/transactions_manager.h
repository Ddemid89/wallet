#ifndef TRANSACTIONSMANAGER_H
#define TRANSACTIONSMANAGER_H

#include <QDate>
#include <QString>
#include "domain.h"
#include <set>
#include <map>

namespace transactions_manager {

class TypeChecker : public TransferVisitorInterface {
public:
    TypeChecker(TransactType type) : type_(type) {}
    void Visit(TransactBase& trs) override;
    void Visit(Transaction& trs) override;
    void Visit(Transfer&) override;
    bool Result();
private:
    bool res_ = true;
    TransactType type_;
};

struct TransactionAdder{
    int acc_idx;
    int cat_idx;
    QDate date;
    Money sum;
    bool inc;
};

struct TransferAdder {
    int from_idx;
    int to_idx;
    Money sum;
    QDate date;
    bool IsValid() const {
        return from_idx != to_idx;
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

class TransactionComp{
public:
    bool operator()(const std::unique_ptr<TransactBase>& a, const std::unique_ptr<TransactBase>& b) const;
};



class TransactionsManager {
public:
    TransactionsManager(LoaderInterface&);
    ~TransactionsManager();

    void AddTransaction(TransactionAdder& trs);
    void AddTransfer(TransferAdder& trs);

    void AddTransaction(TransactionAdder& trs, size_t idx);
    void AddTransfer(TransferAdder& trs, size_t idx);

    QVector<TransactBase*> GetTransacts(TransactType type = TransactType::All,
                                        size_t number = 10, bool late_to_early = true);

    QVector<TransactBase*> GetTransactFiltred (QDate from, QDate to, TransactType type);

    void Save() {
        auto parts = GetRepresentations();

        for (auto& [date, trs] : parts) {
            model_representation::TransactionsPart part;
            part.id = transact_id_;
            part.trs = std::move(trs);
            loader_.Save(date, std::move(part));
        }
    }

    TransactBase* FindTransact(size_t idx);
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
            model_representation::TransType type
                    = static_cast<model_representation::TransType>(trs.type);

            std::unique_ptr<TransactBase> ptr;

            if (type == model_representation::TransType::Transfer) {
                ptr.reset(new Transfer(trs));
            } else {
                ptr.reset(new Transaction(trs, type == model_representation::TransType::Income));
            }

            trns_idx_[trs.id] = ptr.get();
            transacts_.emplace(std::move(ptr));
        }
        transact_id_ = part.id;
    }

    std::map<Date, QVector<model_representation::TransactionRepresentation>>
    GetRepresentations() {
        std::map<Date, QVector<model_representation::TransactionRepresentation>> result;

        for (auto& trs : transacts_) {
            auto repr = trs->GetRepresentation();
            result[repr.date].push_back(std::move(repr));
        }

        return result;
    }

    template<class It>
    QVector<TransactBase*> GetTransacts(It begin, It end, size_t number, TransactType type) {
        TypeChecker tc(type);
        QVector<TransactBase*> result;

        if (type != TransactType::All) {
            auto op = [&tc](const std::unique_ptr<TransactBase>& tr) {
                tr->Visit(tc);
                return tc.Result();
            };

            while(number-- && begin != end) {
                if (op(*begin)) {
                    result.push_back(begin->get());
                }
                ++begin;
            }

            return result;
        } else {
            while(number-- && begin != end) {
                result.push_back(begin->get());
                ++begin;
            }

            return result;
        }
    }

    LoaderInterface& loader_;

    std::set<Date> loaded_;

    std::unordered_map<size_t, TransactBase*> trns_idx_;
    std::set<std::unique_ptr<TransactBase>, TransactionComp> transacts_;

    size_t GetNewId() {
        return transact_id_++;
    }
    size_t transact_id_ = 0;
};
} // namespace transactions_manager

#endif // TRANSACTIONSMANAGER_H
