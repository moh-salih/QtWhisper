#pragma once
#include <QObject>
#include <QString>
#include <thread>
#include <algorithm>
#include <vector>



#include <QMetaType>


namespace QtWhisper {
    Q_NAMESPACE

    enum class Status {
        Idle,
        Loading,
        Ready,
        Error
    };
    Q_ENUM_NS(Status);

    struct Config {
        QString         modelPath;
        bool            useGpu                  = true;
        QString         language                = "en";
        bool            autoDetectLanguage      = false;
        bool            translate               = false;
        int             maxTokens               = 256;
        int             threadCount             = std::min(4, static_cast<int>(std::thread::hardware_concurrency()));
        float           temperature             = 0.0f;
    };

} // namespace QtWhisper

Q_DECLARE_METATYPE(std::vector<float>)

