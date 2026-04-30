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

    enum class Error {
        ModelPathEmpty,
        ModelLoadFailed,
        InferenceFailed
    };
    Q_ENUM_NS(Error);

    enum class Mode {
        Live,
        File
    };

    struct Config {
        QString         modelPath;
        bool            useGpu                  = true;
        QString         language                = "en";
        bool            autoDetectLanguage      = false;
        bool            translate               = false;
        int             maxTokens               = 256;
        int             threadCount             = std::min(4, static_cast<int>(std::thread::hardware_concurrency()));
        float           temperature             = 0.0f;
        Mode            mode                    = Mode::Live;
        bool            autoReload              = true;
    };

    inline QString errorToString(Error error) {
        switch (error) {
            case Error::ModelPathEmpty:   return "Model path is not configured.";
            case Error::ModelLoadFailed:  return "Failed to load Whisper model.";
            case Error::InferenceFailed:  return "Whisper inference failed.";
            default:                      return "Unknown error.";
        }
    }

} // namespace QtWhisper

#ifndef Q_META_TYPE_STD_VECTOR_FLOAT_DECLARED
#define Q_META_TYPE_STD_VECTOR_FLOAT_DECLARED
    Q_DECLARE_METATYPE(std::vector<float>)
#endif
