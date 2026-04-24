#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <QSharedPointer>
#include <QtWhisper/Types.h>

namespace QtWhisper {

    class IEngine : public QObject {
        Q_OBJECT
    public:
        explicit IEngine(QObject *parent = nullptr) : QObject(parent) {}
        virtual ~IEngine() = default;

    public slots:
        virtual void setConfig(QSharedPointer<Config> config) = 0;
        virtual void loadModel()   = 0;
        virtual void unloadModel() = 0;
        virtual void reloadModel() = 0;
        virtual void processWindow(const std::vector<float> &samples) = 0;
        virtual void stop() = 0;
        virtual void reset() = 0;

    signals:
        void segmentTranscribed(const QString &segment);
        void statusChanged(QtWhisper::Status status);
        void processingBusyChanged(bool isProcessing);
        void errorEncountered(const QString &message);
        void reloadRequired();
    };

} // namespace QtWhisper
