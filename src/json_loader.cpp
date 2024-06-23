#include "json_loader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {
QJsonObject TransactionToJson(model_representation::TransactionRepresentation&& transact) {
    QJsonObject res;

    res["date_as_julian_day"] = transact.date.toJulianDay();
    res["from_id"]            = static_cast<int>(transact.from_id);
    res["id"]                 = static_cast<int>(transact.id);
    res["sum"]                = transact.sum;
    res["to_id"]              = static_cast<int>(transact.to_id);
    res["type"]               = transact.type;

    if (transact.description != "") {
        res["description"]        = transact.description;
    }

    return res;
}

QJsonObject TransactionPartToJson(model_representation::TransactionsPart&& part) {
    QJsonObject res;
    QJsonArray trns;
    res["next_id"] = part.id;

    for(model_representation::TransactionRepresentation& trn : part.trs) {
        trns.append(TransactionToJson(std::move(trn)));
    }

    res["transactions"] = std::move(trns);

    return res;
}

model_representation::TransactionRepresentation TransactionFromJson(QJsonValueRef val) {
    QJsonObject obj = val.toObject();
    model_representation::TransactionRepresentation res;

    res.date    = QDate::fromJulianDay(obj.value("date_as_julian_day").toInteger());
    res.from_id = obj.value("from_id").toInteger();
    res.id      = obj.value("id").toInteger();
    res.sum     = obj.value("sum").toInteger();
    res.to_id   = obj.value("to_id").toInteger();
    res.type    = obj.value("type").toInt();

    if (obj.contains("description")) {
        res.description = obj.value("description").toString();
    }

    return res;
}

}

namespace json_loader {

model_representation::TransactionsPart JsonLoader::Load(transactions_manager::Date date) {
    model_representation::TransactionsPart res;

    QString path = GetPath(date);
    QFile file(path);

    if(!file.open(QIODevice::ReadOnly)) {
        res.id = 0;
        qDebug() << "No such file '" << path << "'";
        return res;
    }

    QString raw_json;

    raw_json = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(raw_json.toUtf8());

    QJsonObject obj = doc.object();

    res.id = obj.value("next_id").toInteger();

    QJsonArray array = obj.value("transactions").toArray();

    for (QJsonValueRef trn : array) {
        res.trs.push_back(TransactionFromJson(trn));
    }

    file.close();

    return res;
}

void JsonLoader::Save(transactions_manager::Date date, model_representation::TransactionsPart&& transacts) {
    QString path = GetPath(date);

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open or create file '" << path << "'";
        throw std::runtime_error("Failed to open or create file");
    }

    QJsonDocument doc(TransactionPartToJson(std::move(transacts)));

    file.write(doc.toJson());

    file.close();
}

QString JsonLoader::GetPath(transactions_manager::Date date) {
    return dir_ + "/" + date.String() + ".json";
}

} // namespace json_loader


