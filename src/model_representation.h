#ifndef MODEL_REPRESENTATION_H
#define MODEL_REPRESENTATION_H

#include <QVector>
#include <QDate>

namespace model_representation {

struct AccountRepresentation {
    quint64 id;
    int type;
    QString name;
    qint64 balance_kopek;
    double perc_rate;
    int payday;
    qint64 overdraft;
    bool consider;
    qint64 payment;
    bool deleted;
    QDate next_pay;
};

struct AccountsData {
    quint64 id;
    QVector<AccountRepresentation> accs;
};

struct CategoryRepresentation {
    quint64 id;
    QString name;
    bool inc;
    bool dec;
    quint64 parent_id;
    bool deleted;
};

struct CategoriesData {
    quint64 id;
    QVector<CategoryRepresentation> cats;
};

enum class TransType{
    Income,
    Expense,
    Transfer
};

struct TransactionRepresentation {
    quint64 id;
    int type;
    quint64 from_id;
    quint64 to_id;
    qint64 sum;
    QDate date;
};

struct TransactionsPart {
    qint64 id;
    QVector<TransactionRepresentation> trs;
};


class ModelRepresentation {
public:
    ModelRepresentation (quint64 acc_id, quint64 cat_id, quint64 trs_id,
                         QVector<AccountRepresentation>&& accs,
                         QVector<CategoryRepresentation>&& cats,
                         QVector<TransactionRepresentation>&& trns)
        : acc_id_(acc_id), cat_id_(cat_id), trs_id_(trs_id),
          accs_(std::move(accs)), cats_(std::move(cats)), trns_(std::move(trns)) {}

    quint64 AccId() const {
        return acc_id_;
    }

    quint64 CatId() const {
        return cat_id_;
    }

    quint64 TrsId() const {
        return trs_id_;
    }

    QVector<AccountRepresentation>&& Accs() {
        return std::move(accs_);
    }

    QVector<CategoryRepresentation>&& Cats() {
        return std::move(cats_);
    }

    QVector<TransactionRepresentation>&& Trns() {
        return std::move(trns_);
    }

private:
    quint64 acc_id_;
    quint64 cat_id_;
    quint64 trs_id_;
    QVector<AccountRepresentation>     accs_;
    QVector<CategoryRepresentation>    cats_;
    QVector<TransactionRepresentation> trns_;
};

} // namespace model_representation

#endif // MODEL_REPRESENTATION_H
