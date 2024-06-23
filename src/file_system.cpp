#include "file_system.h"

namespace file_system {

void CheckAndMakeDir(const QString& path) {
    QDir dir(path);

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qDebug() << "Невозможно создать директорию '" + dir.absolutePath() + "'";
        }
    }
}


} // namespace file_system
