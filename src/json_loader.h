#ifndef JSON_LOADER_H
#define JSON_LOADER_H

#include "transactions_manager.h"

namespace json_loader {
class JsonLoader : public transactions_manager::LoaderInterface {
public:
    JsonLoader(QString dir) : dir_{dir} {}
    model_representation::TransactionsPart Load(transactions_manager::Date date) override;
    void Save(transactions_manager::Date date, model_representation::TransactionsPart&& transacts) override;
private:
    QString GetPath(transactions_manager::Date date);

    QString dir_;
};


} // namespace json_loader



#endif // JSON_LOADER_H
