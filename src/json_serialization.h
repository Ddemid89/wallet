#ifndef JSON_SERIALIZATION_H
#define JSON_SERIALIZATION_H

#include "model_representation.h"

namespace json_serialization {
void SerializeAccounts(const QString& file_name, model_representation::AccountsData&& accs);
model_representation::AccountsData DeserializeAccounts(const QString& file);

void SerializeCategories(const QString& file_name, model_representation::CategoriesData&& cats);
model_representation::CategoriesData DeserializeCategories(const QString& file);
} // namespace json_serialization


#endif // JSON_SERIALIZATION_H
