#include "serialization.h"


QDataStream& operator<<(QDataStream& out, const model_representation::AccountRepresentation& acc) {
    out << acc.balance_kopek
        << acc.consider
        << acc.deleted
        << acc.id
        << acc.name
        << acc.next_pay
        << acc.overdraft
        << acc.payday
        << acc.payment
        << acc.perc_rate
        << acc.type;
    return out;
}

QDataStream& operator>>(QDataStream& in, model_representation::AccountRepresentation& acc) {
    in >> acc.balance_kopek
       >> acc.consider
       >> acc.deleted
       >> acc.id
       >> acc.name
       >> acc.next_pay
       >> acc.overdraft
       >> acc.payday
       >> acc.payment
       >> acc.perc_rate
       >> acc.type;
    return in;
}

QDataStream& operator<<(QDataStream& out, const model_representation::AccountsData& accs){
    out << accs.id << accs.accs;
    return out;
}

QDataStream& operator>>(QDataStream& in, model_representation::AccountsData& accs){
    in >> accs.id >> accs.accs;
    return in;
}



model_representation::AccountsData model_serialization::DeserializeAccounts(const QString& file) {
    model_representation::AccountsData accs;
    QFile acc_file(file);
    if(!acc_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file '" + file + "'";
        accs.id = 1;
        return accs;
    }

    QDataStream in(&acc_file);

    in >> accs;
    acc_file.close();

    return accs;
}

void model_serialization::SerializeAccounts(const QString& file, model_representation::AccountsData&& accs) {
    QFile acc_file(file);
    if(!acc_file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file '" + file + "'";
        return;
    }

    QDataStream out(&acc_file);

    out << accs;
    acc_file.close();
}

QDataStream& operator<<(QDataStream& out, const model_representation::CategoryRepresentation& cat) {
    out << cat.dec
        << cat.deleted
        << cat.id
        << cat.inc
        << cat.name
        << cat.parent_id;
    return out;
}

QDataStream& operator>>(QDataStream& in, model_representation::CategoryRepresentation& cat) {
    in >> cat.dec
       >> cat.deleted
       >> cat.id
       >> cat.inc
       >> cat.name
       >> cat.parent_id;
    return in;
}

QDataStream& operator<<(QDataStream& out, const model_representation::CategoriesData& cats) {
    out << cats.id << cats.cats;
    return out;
}

QDataStream& operator>>(QDataStream& in, model_representation::CategoriesData& cats) {
    in >> cats.id >> cats.cats;
    return in;
}

void model_serialization::SerializeCategories(const QString& file, model_representation::CategoriesData&& cats) {
    QFile cat_file(file);
    if(!cat_file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file '" + file + "'";
        return;
    }

    QDataStream out(&cat_file);

    out << cats;
    cat_file.close();
}

model_representation::CategoriesData model_serialization::DeserializeCategories(const QString& file) {
    model_representation::CategoriesData cats;
    QFile cat_file(file);
    if(!cat_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file '" + file + "'";
        cats.id = 1;
        return cats;
    }

    QDataStream in(&cat_file);

    in >> cats;
    cat_file.close();

    return cats;
}
