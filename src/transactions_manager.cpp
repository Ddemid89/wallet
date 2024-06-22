#include "transactions_manager.h"

namespace transactions_manager {

TransactionsManager::TransactionsManager(LoaderInterface& loader) : loader_(loader) {
    Load(QDate::currentDate().addYears(-1), QDate::currentDate());
}

TransactionsManager::~TransactionsManager() {

}

void TransactionsManager::AddTransaction(TransactionAdder& trs) {
    size_t new_id = GetNewId();

    AddTransaction(trs, new_id);
}

void TransactionsManager::AddTransaction(TransactionAdder &trs, size_t idx) {
    Load(trs.date);

    Transaction new_trns(idx, static_cast<size_t>(trs.from_idx),
                             static_cast<size_t>(trs.to_idx),
                             trs.type, trs.date, trs.sum, trs.description);

    auto pair = transacts_.insert(new_trns);
    const Transaction* addr = &(*pair.first);
    trns_idx_[idx] = addr;
}

QVector<const Transaction*> TransactionsManager::GetTransacts(TransactShowType type, size_t number, bool late_to_early) {
    if (late_to_early) {
        return GetTransacts(transacts_.cbegin(), transacts_.cend(), number, type);
    } else {
        return GetTransacts(transacts_.crbegin(), transacts_.crend(), number, type);
    }
}

QVector<const Transaction*> TransactionsManager::GetTransactFiltred(QDate from, QDate to, TransactShowType type) {
    Load(from, to);

    QVector<const Transaction*> result;

    Transaction l_bound(0, 0, 0, TransactionType::Income, from, 0);

    auto it = transacts_.lower_bound(l_bound);

    TransactionMatcher matcher{type};

    while(it != transacts_.end() && it->Date() <= to) {
        if (matcher.Check(it->Type())) {
            result.push_back(&(*it));
        }
        ++it;
    }

    return result;
}

const Transaction* TransactionsManager::FindTransact(size_t idx) {
    std::unique_ptr<Transaction> tmp;
    return trns_idx_.at(idx);
}

void TransactionsManager::DeleteTransact(size_t idx) {
    auto trns = trns_idx_.at(idx);
    auto it = transacts_.find(*trns);
    auto idx_it = trns_idx_.find(idx);

    if (it != transacts_.end() || idx_it == trns_idx_.end()) {
        transacts_.erase(it);
        trns_idx_.erase(idx_it);
    } else {
        throw std::runtime_error("Something wrong in TransactionManager::DeleteTransact");
    }
}

bool Date::operator<(Date other) const {
    return year < other.year
            || (year == other.year && month < other.month);
}

bool Date::operator<=(Date other) const {
    return year < other.year
            || (year == other.year && !(month > other.month));
}

QString Date::String() const {
    return QString::number(month) + "_" + QString::number(year);
}

} // namespace transactions_manager
