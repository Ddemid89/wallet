#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "model_representation.h"
#include <QFile>
#include <QDataStream>
#include <QVector>

QDataStream& operator<<(QDataStream& out, const model_representation::AccountRepresentation&);
QDataStream& operator>>(QDataStream& in, model_representation::AccountRepresentation&);
QDataStream& operator<<(QDataStream& out, const model_representation::AccountsData&);
QDataStream& operator>>(QDataStream& in, model_representation::AccountsData&);

QDataStream& operator<<(QDataStream& out, const model_representation::CategoryRepresentation&);
QDataStream& operator>>(QDataStream& in, model_representation::CategoryRepresentation&);
QDataStream& operator<<(QDataStream& out, const model_representation::CategoriesData&);
QDataStream& operator>>(QDataStream& in, model_representation::CategoriesData&);

namespace model_serialization {

void SerializeAccounts(const QString& file, model_representation::AccountsData&& accs);
model_representation::AccountsData DeserializeAccounts(const QString& file);

void SerializeCategories(const QString& file, model_representation::CategoriesData&& cats);
model_representation::CategoriesData DeserializeCategories(const QString& file);

} // namespace model_serialization

#endif // SERIALIZATION_H
