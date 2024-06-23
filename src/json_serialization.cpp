#include "json_serialization.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

namespace str {
const auto id       = "id";
const auto next_id  = "next_id";
const auto accs     = "accounts";
const auto cats     = "categories";
const auto jsn      = ".json";
const auto name     = "name";
const auto deleted  = "deleted";
const auto acc_type = "type";
const auto cat_type = "type_inc_dec";
const auto parent   = "parent_id";
const auto balance  = "balance";
const auto consider = "consider";
const auto overdrft = "overdraft";
const auto payday   = "payday";
const auto payment  = "payment";
const auto percent  = "percent_rate";
const auto next_pay = "next_pay_as_julian_day";
}

namespace {

QJsonObject AccToJson(model_representation::AccountRepresentation&& acc) {
    QJsonObject res;

    res[str::balance]  = static_cast<int>(acc.balance_kopek);
    res[str::consider] = acc.consider;
    res[str::deleted]  = acc.deleted;
    res[str::id]       = static_cast<int>(acc.id);
    res[str::name]     = acc.name;

    res[str::next_pay] = acc.next_pay.toJulianDay();

    res[str::overdrft]    = static_cast<int>(acc.overdraft);
    res[str::payday]       = acc.payday;
    res[str::payment]      = static_cast<int>(acc.payment);
    res[str::percent] = acc.perc_rate;
    res[str::acc_type]         = acc.type;

    return res;
}

QJsonObject CatToJson(model_representation::CategoryRepresentation&& cat) {
    QJsonObject res;

    res[str::deleted]  = cat.deleted;
    res[str::id]       = static_cast<int>(cat.id);
    res[str::cat_type] = static_cast<int>(cat.inc_dec);
    res[str::name]     = cat.name;
    res[str::parent]   = static_cast<int>(cat.parent_id);

    return res;
}

QJsonObject AccsDataToJson(model_representation::AccountsData&& data) {
    QJsonObject res;

    res[str::next_id] = static_cast<int>(data.id);

    QJsonArray arr;

    for (auto& acc : data.accs) {
        arr.append(AccToJson(std::move(acc)));
    }

    res[str::accs] = arr;

    return res;
}

QJsonObject CatsDataToJson(model_representation::CategoriesData&& data) {
    QJsonObject res;

    res[str::next_id] = static_cast<int>(data.id);

    QJsonArray arr;

    for (auto& cat : data.cats) {
        arr.append(CatToJson(std::move(cat)));
    }

    res[str::cats] = arr;

    return res;
}

model_representation::AccountRepresentation AccFromJson(QJsonValueRef& ref) {
    QJsonObject obj = ref.toObject();

    model_representation::AccountRepresentation res;

    res.balance_kopek = obj.value(str::balance).toInt();
    res.consider      = obj.value(str::consider).toBool();
    res.deleted       = obj.value(str::deleted).toBool();
    res.id            = obj.value(str::id).toInteger();
    res.name          = obj.value(str::name).toString();
    res.next_pay      = QDate::fromJulianDay(obj.value(str::next_pay).toInteger());
    res.overdraft     = obj.value(str::overdrft).toInteger();
    res.payday        = obj.value(str::payday).toInt();
    res.payment       = obj.value(str::payment).toInteger();
    res.perc_rate     = obj.value(str::percent).toDouble();
    res.type          = obj.value(str::acc_type).toInt();

    return res;
}

model_representation::CategoryRepresentation CatFromJson(QJsonValueRef& ref) {
    QJsonObject obj = ref.toObject();

    model_representation::CategoryRepresentation res;

    res.deleted   = obj.value(str::deleted).toBool();
    res.id        = obj.value(str::id).toInteger();
    res.inc_dec   = obj.value(str::cat_type).toInteger();
    res.name      = obj.value(str::name).toString();
    res.parent_id = obj.value(str::parent).toInteger();

    return res;
}

model_representation::AccountsData AccsDataFromJson(QJsonObject&& obj) {
    model_representation::AccountsData res;

    res.id = obj.value(str::next_id).toInteger();

    auto arr = obj.value(str::accs).toArray();

    for (auto acc : arr) {
        res.accs.push_back(AccFromJson(acc));
    }

    return res;
}

model_representation::CategoriesData CatsDataFromJson(QJsonObject&& obj) {
    model_representation::CategoriesData res;

    res.id = obj.value(str::next_id).toInteger();

    auto arr = obj.value(str::cats).toArray();

    for (auto cat : arr) {
        res.cats.push_back(CatFromJson(cat));
    }

    return res;
}

void WriteJsonToFile(const QString& file_name, QJsonObject&& obj) {
    QFile file(file_name + str::jsn);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open or create file '" << file_name + str::jsn << "'";
        throw std::runtime_error("Failed to open or create file");
    }

    QJsonDocument doc(obj);

    file.write(doc.toJson());

    file.close();
}

QJsonObject ReadJsonFromFile(const QString& file_name) {
    QFile file(file_name + str::jsn);

    QJsonObject res;

    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file '" + file_name + ".json'";
        res[str::next_id] = 1;
        return res;
    }

    QString raw_json;

    raw_json = file.readAll();

    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(raw_json.toUtf8());

    return doc.object();
}

} // namespace

namespace json_serialization {

void SerializeAccounts(const QString& file_name, model_representation::AccountsData&& accs) {
    WriteJsonToFile(file_name, AccsDataToJson(std::move(accs)));
}

model_representation::AccountsData DeserializeAccounts(const QString& file) {
    QJsonObject obj = ReadJsonFromFile(file);
    return AccsDataFromJson(std::move(obj));
}

void SerializeCategories(const QString& file_name, model_representation::CategoriesData&& cats) {
    WriteJsonToFile(file_name, CatsDataToJson(std::move(cats)));
}

model_representation::CategoriesData DeserializeCategories(const QString& file) {
    QJsonObject obj = ReadJsonFromFile(file);
    return CatsDataFromJson(std::move(obj));
}

} // namespace json_serialization

